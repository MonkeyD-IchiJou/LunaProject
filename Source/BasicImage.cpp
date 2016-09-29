#include "BasicImage.h"
#include "DebugLog.h"
#include "Renderer.h"

#include <stb_image.h>

namespace luna
{

	BasicImage::BasicImage(unsigned char* pixels, const int& texwidth, const int& texheight, const int& texchannels) :
		m_pixels(pixels),
		m_texwidth(texwidth),
		m_texheight(texheight),
		m_texchannels(texchannels)
	{
		if (!m_pixels)
		{
			DebugLog::throwEx("invalid pixels image");
		}

		m_imageTotalSize = m_texwidth * m_texheight * 4; // rgba

		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();

		CreateImage();
	}

	void BasicImage::CreateImage()
	{
		m_stagebuff.format = VK_FORMAT_R8G8B8A8_UNORM;
		m_stagebuff.tiling = VK_IMAGE_TILING_LINEAR;
		m_stagebuff.imageusage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		CreateImageBuff_(m_stagebuff, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		m_imagebuff.format = VK_FORMAT_R8G8B8A8_UNORM;
		m_imagebuff.tiling = VK_IMAGE_TILING_OPTIMAL;
		m_imagebuff.imageusage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		CreateImageBuff_(m_imagebuff, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	void BasicImage::CreateImageBuff_(VulkanImageBufferData & buff, const VkMemoryPropertyFlags & memoryproperties)
	{
		// create the image buffer for it
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = m_texwidth;
		imageInfo.extent.height = m_texheight;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = buff.format;
		imageInfo.tiling = buff.tiling; // we will be using VK_IMAGE_TILING_OPTIMAL for the final image
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED; // shld use preinitialized layout for texture (to fill with data) 
		imageInfo.usage = buff.imageusage;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; // only relevant for images that will be used as attachments
		imageInfo.flags = 0;
		DebugLog::EC(vkCreateImage(m_logicaldevice, &imageInfo, nullptr, &buff.buffer));

		// query Image Memory info
		VkMemoryRequirements memRequirements{};
		vkGetImageMemoryRequirements(m_logicaldevice, buff.buffer, &memRequirements);
		buff.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memRequirements.memoryTypeBits,
			memoryproperties); // find the best heap in the GPU to store at
		buff.BufferTotalSize = m_imageTotalSize;
		buff.RequirementSizeInDeviceMem = memRequirements.size;
		buff.RequirementAlignmentInDeviceMem = memRequirements.alignment;
	}

	void BasicImage::TransitionStagedAndMainImageLayout()
	{
		// need to transition the layout of both images before copying each other
		TransitionImageLayout_(m_stagebuff.buffer, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		TransitionImageLayout_(m_imagebuff.buffer, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
	}

	const void BasicImage::MapToDeviceMemory(const VkDeviceMemory & devicememory)
	{
		if (m_pixels)
		{
			// map the data into staging image buffer
			void *data = nullptr;
			vkMapMemory(m_logicaldevice, devicememory, m_imageBufferOffset, m_stagebuff.RequirementSizeInDeviceMem, 0, &data);
			memcpy(data, m_pixels, (size_t)m_stagebuff.RequirementSizeInDeviceMem);
			vkUnmapMemory(m_logicaldevice, devicememory);

			// can free the pixels once copied
			stbi_image_free(m_pixels);
		}
	}

	void BasicImage::CopyImage(const VkCommandBuffer& commandbuffer)
	{
		// create the command buffer and start recording it
		VkImageSubresourceLayers subResource{};
		subResource.aspectMask					= VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.baseArrayLayer				= 0;
		subResource.mipLevel					= 0;
		subResource.layerCount					= 1;

		VkImageCopy region{};
		region.srcSubresource					= subResource;
		region.dstSubresource					= subResource;
		region.srcOffset						= { 0, 0, 0 };
		region.dstOffset						= { 0, 0, 0 };
		region.extent.width						= m_texwidth;
		region.extent.height					= m_texheight;
		region.extent.depth						= 1;

		vkCmdCopyImage(commandbuffer, m_stagebuff.buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_imagebuff.buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	void BasicImage::CreateImageView(const VkFormat& format, const VkImageAspectFlags& aspectFlags)
	{
		// to be able to start sampling in the shader
		TransitionImageLayout_(m_imagebuff.buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

		// some clean up before create the image view
		if (m_stagebuff.buffer != VK_NULL_HANDLE)
		{
			vkDestroyImage(m_logicaldevice, m_stagebuff.buffer, nullptr);
			m_stagebuff.buffer = VK_NULL_HANDLE;
		}

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_imagebuff.buffer;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		DebugLog::EC(vkCreateImageView(m_logicaldevice, &viewInfo, nullptr, &m_imageview));
	}

	void BasicImage::TransitionImageLayout_(VkImage srcimage, const VkImageLayout & oldLayout, const VkImageLayout & newLayout, const VkImageAspectFlags & aspectMask, 
		const VkAccessFlags & srcAccessMask, const VkAccessFlags & dstAccessMask)
	{
		VkCommandPool commandpool = VK_NULL_HANDLE;

		// create the temp command pool
		VkCommandPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createinfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		createinfo.queueFamilyIndex = Renderer::getInstance()->GetQueueFamilyIndices().graphicsFamily;
		vkCreateCommandPool(m_logicaldevice, &createinfo, nullptr, &commandpool);

		// create the command buffer and start recording it
		VkCommandBuffer commandbuffer = BeginSingleTimeCommands_(m_logicaldevice, commandpool);

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = srcimage;
		barrier.subresourceRange.aspectMask	= aspectMask;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer	= 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.srcAccessMask = srcAccessMask; 
		barrier.dstAccessMask = dstAccessMask; 

		vkCmdPipelineBarrier(
			commandbuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		// then submit this to the graphics queue to execute it
		EndSingleTimeCommands_(m_logicaldevice, commandbuffer);

		// clean up
		vkDestroyCommandPool(m_logicaldevice, commandpool, nullptr);
	}

	VkCommandBuffer BasicImage::BeginSingleTimeCommands_(const VkDevice & logicaldevice, VkCommandPool commandPool)
	{
		VkCommandBuffer commandbuffer = VK_NULL_HANDLE;

		// allocate the temporary command buffer
		VkCommandBufferAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocinfo.commandPool = commandPool;
		allocinfo.commandBufferCount = 1;
		allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vkAllocateCommandBuffers(logicaldevice, &allocinfo, &commandbuffer);

		// immediately start recording this command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandbuffer, &beginInfo);

		return commandbuffer;
	}

	void BasicImage::EndSingleTimeCommands_(const VkDevice & logicaldevice, VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		// now execute the command buffer
		VkSubmitInfo submitinfo{};
		submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(Renderer::getInstance()->GetGraphicQueue(), 1, &submitinfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(Renderer::getInstance()->GetGraphicQueue());
	}

	BasicImage::~BasicImage()
	{
		if (m_stagebuff.buffer != VK_NULL_HANDLE)
		{
			vkDestroyImage(m_logicaldevice, m_stagebuff.buffer, nullptr);
			m_stagebuff.buffer = VK_NULL_HANDLE;
		}

		if (m_imagebuff.buffer != VK_NULL_HANDLE)
		{
			vkDestroyImage(m_logicaldevice, m_imagebuff.buffer, nullptr);
			m_imagebuff.buffer = VK_NULL_HANDLE;
		}

		if (m_imageview != VK_NULL_HANDLE)
		{
			vkDestroyImageView(m_logicaldevice, m_imageview, nullptr);
			m_imageview = VK_NULL_HANDLE;
		}
	}
}
