#include "VulkanImageBufferObject.h"
#include "Renderer.h"
#include "DebugLog.h"

namespace luna
{

	VulkanImageBufferObject::VulkanImageBufferObject()
	{
		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();
	}


	VulkanImageBufferObject::~VulkanImageBufferObject()
	{
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

	void VulkanImageBufferObject::CreateImageBuff_(VulkanImageBufferData & buff, const VkMemoryPropertyFlags & memoryproperties)
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
		buff.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memRequirements.memoryTypeBits, memoryproperties); // find the best heap in the GPU to store at
		buff.BufferTotalSize = m_imageTotalSize;
		buff.RequirementSizeInDeviceMem = memRequirements.size;
		buff.RequirementAlignmentInDeviceMem = memRequirements.alignment;
	}

	void VulkanImageBufferObject::TransitionImageLayout_(VkImage srcimage, const VkImageLayout & oldLayout, const VkImageLayout & newLayout, const VkImageAspectFlags & aspectMask, 
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

	VkCommandBuffer VulkanImageBufferObject::BeginSingleTimeCommands_(const VkDevice & logicaldevice, VkCommandPool commandPool)
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

	void VulkanImageBufferObject::EndSingleTimeCommands_(const VkDevice & logicaldevice, VkCommandBuffer commandBuffer)
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
}
