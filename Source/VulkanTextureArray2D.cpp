#include "VulkanTextureArray2D.h"
#include <gli/gli.hpp>
#include "Renderer.h"
#include "DebugLog.h"
#include "Global.h"

namespace luna
{
	VulkanTextureArray2D::VulkanTextureArray2D(const std::string& filename)
	{
#if defined(__ANDROID__)

		// Textures are stored inside the apk on Android (compressed)
		// So they need to be loaded via the asset manager
		AAsset* asset = AAssetManager_open(global::androidApplication->activity->assetManager, filename.c_str(), AASSET_MODE_STREAMING);
		if (asset == nullptr)
			DebugLog::printFF("failed to open file!");

		size_t size = AAsset_getLength(asset);
		if(size <= 0)
			DebugLog::printFF("file size is less than zero");

		void *textureData = malloc(size);
		AAsset_read(asset, textureData, size);
		AAsset_close(asset);

		gli::texture2d_array tex(gli::load((const char*)textureData, size));

#else
		// load the texture first
		gli::texture2d_array tex(gli::load(filename));
#endif
		m_layers = static_cast<uint32_t>(tex.layers());

		// if there is only 1 layers, error 
		if (m_layers < 2)
		{
			DebugLog::throwEx("Texture less than 2 layers");
		}

		m_format = (VkFormat)tex.format();
		m_texsize = tex.size();
		m_texwidth = static_cast<uint32_t>(tex.extent().x);
		m_texheight = static_cast<uint32_t>(tex.extent().y);
		m_mipLevels = static_cast<uint32_t>(tex.levels());
		
		// Create a host-visible staging buffer that contains the raw image data
		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingMemory = VK_NULL_HANDLE;
		{
			VkMemoryRequirements memReqs{};
			CreateStagingBuffer_(m_texsize, stagingBuffer, stagingMemory, memReqs);

			// Copy texture data into staging buffer
			void *data = nullptr;
			DebugLog::EC(vkMapMemory(m_logicaldevice, stagingMemory, 0, memReqs.size, 0, &data));
			memcpy(data, tex.data(), m_texsize);
			vkUnmapMemory(m_logicaldevice, stagingMemory);
		}

		// create the image buffers and bind it to the allocated memory
		// Create optimal tiled target image
		{
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.extent = {m_texwidth, m_texheight, 1};
			imageInfo.format = m_format;
			imageInfo.mipLevels = m_mipLevels;
			imageInfo.arrayLayers = m_layers;
			DebugLog::EC(vkCreateImage(m_logicaldevice, &imageInfo, nullptr, &m_image));

			// allocate the memory to the image buffer
			VkMemoryRequirements memReqs{};
			vkGetImageMemoryRequirements(m_logicaldevice, m_image, &memReqs);

			VkMemoryAllocateInfo memAllocInfo{};
			memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memAllocInfo.allocationSize = memReqs.size;
			memAllocInfo.memoryTypeIndex = Renderer::getInstance()->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			DebugLog::EC(vkAllocateMemory(m_logicaldevice, &memAllocInfo, nullptr, &m_devicememory));

			// bind the image buffer
			DebugLog::EC(vkBindImageMemory(m_logicaldevice, m_image, m_devicememory, 0));
		}

		// Setup buffer copy regions for each mip level
		std::vector<VkBufferImageCopy> bufferCopyRegions;
		uint32_t offset = 0;

		// check if all array layers have the same dimensions
		bool sameDims = true;
		for (uint32_t layer = 0; layer < m_layers; layer++)
		{
			if (tex[layer].extent().x != m_texwidth ||
				tex[layer].extent().y != m_texheight)
			{
				sameDims = false;
				break;
			}
		}

		// If all layers of the texture array have the same dimensions, we only need to do one copy
		if (sameDims)
		{
			bufferCopyRegions.resize(m_mipLevels);

			for (uint32_t level = 0; level < m_mipLevels; level++)
			{
				VkBufferImageCopy& copyregion = bufferCopyRegions[level];
				copyregion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyregion.imageSubresource.mipLevel = level;
				copyregion.imageSubresource.baseArrayLayer = 0; // shld be 0
				copyregion.imageSubresource.layerCount = m_layers; // total num of layers tht it has
				copyregion.imageExtent.width = static_cast<uint32_t>(tex.extent(level).x);
				copyregion.imageExtent.height = static_cast<uint32_t>(tex.extent(level).y);
				copyregion.imageExtent.depth = 1;
				copyregion.bufferOffset = offset;

				offset += static_cast<uint32_t>(tex.size(level));
			}
		}
		else
		{
			// If dimensions differ, copy layer by layer and pass offsets
			bufferCopyRegions.resize(m_layers * m_mipLevels);

			for (uint32_t layer = 0; layer < m_layers; ++layer)
			{
				for (uint32_t level = 0; level < m_mipLevels; level++)
				{
					uint32_t index = (layer * m_mipLevels) + level;
					VkBufferImageCopy& copyregion = bufferCopyRegions[index];

					copyregion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					copyregion.imageSubresource.mipLevel = level;
					copyregion.imageSubresource.baseArrayLayer = layer;
					copyregion.imageSubresource.layerCount = 1; // one by one
					copyregion.imageExtent.width = static_cast<uint32_t>(tex[layer].extent(level).x);
					copyregion.imageExtent.height = static_cast<uint32_t>(tex[layer].extent(level).y);
					copyregion.imageExtent.depth = 1;
					copyregion.bufferOffset = offset;

					offset += static_cast<uint32_t>(tex[layer].size(level));
				}
			}
		}

		// The sub resource range describes the regions of the image we will be transition
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Image only contains color data
		subresourceRange.baseMipLevel = 0; // Start at first mip level
		subresourceRange.levelCount = m_mipLevels; // We will transition on all mip levels
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = m_layers;

		// copy buffers data into image buffer
		CopyBufferToImageBuffer_(bufferCopyRegions, m_image, stagingBuffer, stagingMemory, subresourceRange);
	

		// image view creation
		// create the image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
		viewInfo.format = m_format;
		viewInfo.subresourceRange = subresourceRange;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		DebugLog::EC(vkCreateImageView(m_logicaldevice, &viewInfo, nullptr, &m_imageview));
	
		// create the sampler for this image
		m_sampler = CreateSampler_(true,  static_cast<float>(m_mipLevels), true, 16.f);
	}

	VulkanTextureArray2D::~VulkanTextureArray2D()
	{
		if (m_image != VK_NULL_HANDLE)
		{
			vkDestroyImage(m_logicaldevice, m_image, nullptr);
			m_image = VK_NULL_HANDLE;
		}

		if (m_imageview != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_logicaldevice, m_imageview, nullptr);
			m_imageview = VK_NULL_HANDLE;
		}

		if (m_sampler != VK_NULL_HANDLE)
		{
			vkDestroySampler(m_logicaldevice, m_sampler, nullptr);
			m_sampler = VK_NULL_HANDLE;
		}

		if (m_devicememory != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_logicaldevice, m_devicememory, nullptr);
			m_devicememory = VK_NULL_HANDLE;
		}
	}
}