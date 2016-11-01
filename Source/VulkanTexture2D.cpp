#include "VulkanTexture2D.h"
#include <gli/gli.hpp>
#include "Renderer.h"
#include "DebugLog.h"
#include "Global.h"

namespace luna
{
	VulkanTexture2D::VulkanTexture2D(const std::string& filename)
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

		gli::texture2d tex(gli::load((const char*)textureData, size));

#else
		// load the texture first
		gli::texture2d tex(gli::load(filename));
#endif
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
		// create the image buffer for it
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
			imageInfo.arrayLayers = 1;
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
		std::vector<VkBufferImageCopy> bufferCopyRegions(m_mipLevels);
		uint32_t offset = 0;
		for (uint32_t level = 0; level < m_mipLevels; level++)
		{
			VkBufferImageCopy& copyregion = bufferCopyRegions[level];
			copyregion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			copyregion.imageSubresource.mipLevel = level;
			copyregion.imageSubresource.baseArrayLayer = 0; // shld be 0
			copyregion.imageSubresource.layerCount = 1;
			copyregion.imageExtent.width = static_cast<uint32_t>(tex.extent(level).x);
			copyregion.imageExtent.height = static_cast<uint32_t>(tex.extent(level).y);
			copyregion.imageExtent.depth = 1;
			copyregion.bufferOffset = offset;

			offset += static_cast<uint32_t>(tex.size(level));
		}

		// The sub resource range describes the regions of the image we will be transition
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; // Image only contains color data
		subresourceRange.baseMipLevel = 0; // Start at first mip level
		subresourceRange.levelCount = m_mipLevels; // We will transition on all mip levels
		subresourceRange.layerCount = 1; // The 2D texture only has one layer

		// copy buffers data into image buffer
		CopyBufferToImageBuffer_(bufferCopyRegions, m_image, stagingBuffer, stagingMemory, subresourceRange);
	
		// image view creation
		// create the image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_format;
		viewInfo.subresourceRange = subresourceRange;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		DebugLog::EC(vkCreateImageView(m_logicaldevice, &viewInfo, nullptr, &m_imageview));
	
		m_sampler = CreateSampler_(true,  static_cast<float>(m_mipLevels), true, 16.f);
	}

	VulkanTexture2D::VulkanTexture2D(const uint32_t& width, const uint32_t& height, 
		const VkFormat& format, const VkImageUsageFlags& usage, const VkImageAspectFlags& aspectMask, const VkImageLayout& imagelayout)
	{
		m_format = format; 
		m_texsize = 0; // determined by memReqs;
		m_texwidth = width;
		m_texheight = height;
		m_mipLevels = 1;

		// create the image buffer for it
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = usage;
		imageInfo.extent = {m_texwidth, m_texheight, 1};
		imageInfo.mipLevels = m_mipLevels;
		imageInfo.arrayLayers = 1;
		imageInfo.format = m_format;
		DebugLog::EC(vkCreateImage(m_logicaldevice, &imageInfo, nullptr, &m_image));

		// query Image Memory info
		VkMemoryRequirements memReqs{};
		vkGetImageMemoryRequirements(m_logicaldevice, m_image, &memReqs);

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.memoryTypeIndex = Renderer::getInstance()->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		memAllocInfo.allocationSize = memReqs.size;
		vkAllocateMemory(m_logicaldevice, &memAllocInfo, nullptr, &m_devicememory);

		vkBindImageMemory(m_logicaldevice, m_image, m_devicememory, 0);

		// transition the image layout
		// create the command buffer and start recording it
		VkCommandPool commandpool = VK_NULL_HANDLE;
		VkCommandBuffer commandbuffer = BeginSingleTimeCommands_(commandpool);

		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = aspectMask; 
		subresourceRange.baseMipLevel = 0; // Start at first mip level
		subresourceRange.levelCount = 1; // We will transition on all mip levels
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1; // The 2D texture only has one layer

		// make sure the image layout is suitable for any usage later 
		TransitionImageLayout_(
			commandbuffer,
			m_image,
			VK_IMAGE_LAYOUT_UNDEFINED, imagelayout,
			subresourceRange
		);

		// then submit this to the graphics queue to execute it
		EndSingleTimeCommands_(commandbuffer, Renderer::getInstance()->GetGraphicQueue());

		// clean up
		vkDestroyCommandPool(m_logicaldevice, commandpool, nullptr);

		// image view creation
		// create the image view
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_format;
		viewInfo.subresourceRange = subresourceRange;
		viewInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
		DebugLog::EC(vkCreateImageView(m_logicaldevice, &viewInfo, nullptr, &m_imageview));
	
		m_sampler = CreateSampler_();
	}

	VulkanTexture2D::~VulkanTexture2D()
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