#include "VulkanImageBufferObject.h"
#include "Renderer.h"
#include "DebugLog.h"

namespace luna
{
	VkDevice VulkanImageBufferObject::m_logicaldevice = VK_NULL_HANDLE;

	VulkanImageBufferObject::VulkanImageBufferObject()
	{
		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();
	}

	VulkanImageBufferObject::~VulkanImageBufferObject()
	{
	}

	void VulkanImageBufferObject::CreateStagingBuffer_(const VkDeviceSize& size, VkBuffer& stagingbuffer, VkDeviceMemory& stagingmemory, VkMemoryRequirements& memReqs)
	{
		VkBufferCreateInfo bufferCreateInfo{};
		bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		bufferCreateInfo.size = size;
		DebugLog::EC(vkCreateBuffer(m_logicaldevice, &bufferCreateInfo, nullptr, &stagingbuffer));

		// Get memory requirements for the staging buffer (alignment, memory type bits)
		vkGetBufferMemoryRequirements(m_logicaldevice, stagingbuffer, &memReqs);

		VkMemoryAllocateInfo memAllocInfo{};
		memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memAllocInfo.allocationSize = memReqs.size;
		memAllocInfo.memoryTypeIndex = Renderer::getInstance()->findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
		DebugLog::EC(vkAllocateMemory(m_logicaldevice, &memAllocInfo, nullptr, &stagingmemory));

		// bind the buffer
		DebugLog::EC(vkBindBufferMemory(m_logicaldevice, stagingbuffer, stagingmemory, 0));
	}

	void VulkanImageBufferObject::TransitionImageLayout_(const VkCommandBuffer& buffer, VkImage srcimage, const VkImageLayout & oldLayout, const VkImageLayout & newLayout, 
		const VkImageSubresourceRange & subresourceRange)
	{
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.image = srcimage;
		barrier.subresourceRange = subresourceRange;

		// Only sets masks for layouts used in this example
		// For a more complete version that can be used with other layouts see vkTools::setImageLayout

		// Source layouts (old)
		switch (oldLayout)
		{
		case VK_IMAGE_LAYOUT_UNDEFINED:
			// Only valid as initial layout, memory contents are not preserved
			// Can be accessed directly, no source dependency required
			barrier.srcAccessMask = 0;
			break;
		case VK_IMAGE_LAYOUT_PREINITIALIZED:
			// Only valid as initial layout for linear images, preserves memory contents
			// Make sure host writes to the image have been finished
			barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Old layout is transfer destination
			// Make sure any writes to the image have been finished
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Shader write (sampler, input attachment)
			barrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
			break;

		default:
			break;
		}

		// Target layouts (new)
		switch (newLayout)
		{
		case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
			// Transfer source (copy, blit)
			// Make sure any reads from the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			break;
		case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
			// Transfer destination (copy, blit)
			// Make sure any writes to the image have been finished
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			break;
		case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
			// Shader read (sampler, input attachment)
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
			break;

		case VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL:
			// depth image read and write
			barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
			break;

		case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
			// color image read and write
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			break;

		default:
			break;
		}

		vkCmdPipelineBarrier(
			buffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
		const VkCommandBuffer & commandbuffer, const VkImage& image, 
		const VkImageLayout& oldlayout, const VkImageLayout& newlayout, 
		VkAccessFlags srcaccessflag, VkAccessFlags dstaccessflag,
		VkPipelineStageFlags srcpipelinestage, VkPipelineStageFlags dstpipelinestage,
		VkImageAspectFlags aspectMask
	)
	{
		// make sure it is optimal for the swap chain images
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldlayout;
		barrier.newLayout = newlayout;
		barrier.srcAccessMask = srcaccessflag;
		barrier.dstAccessMask = dstaccessflag;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = aspectMask;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer	= 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.image = image;

		vkCmdPipelineBarrier(
			commandbuffer,
			srcpipelinestage, // begin to work here
			dstpipelinestage, // must make sure the work end at this stage
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void VulkanImageBufferObject::CopyBufferToImageBuffer_(const std::vector<VkBufferImageCopy>& bufferCopyRegions, const VkImage& srcImage, VkBuffer& stagingbuffer, 
		VkDeviceMemory& stagingmemory, const VkImageSubresourceRange& subresourceRange)
	{
		// Image barrier for optimal image

		// create the command buffer and start recording it
		VkCommandPool commandpool = VK_NULL_HANDLE;
		VkCommandBuffer commandbuffer = BeginSingleTimeCommands_(commandpool);

		TransitionImageLayout_(
			commandbuffer,
			srcImage, 
			VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			subresourceRange
		);

		vkCmdCopyBufferToImage(
			commandbuffer, 
			stagingbuffer, 
			srcImage,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			static_cast<uint32_t>(bufferCopyRegions.size()), 
			bufferCopyRegions.data()
		);

		// Change texture image layout to shader read after all mip levels have been copied
		TransitionImageLayout_(
			commandbuffer,
			srcImage, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, 
			subresourceRange
		);

		// then submit this to the graphics queue to execute it
		EndSingleTimeCommands_(commandbuffer, Renderer::getInstance()->GetGraphicQueue());

		// clean up
		vkDestroyCommandPool(m_logicaldevice, commandpool, nullptr);
		vkFreeMemory(m_logicaldevice, stagingmemory, nullptr);
		vkDestroyBuffer(m_logicaldevice, stagingbuffer, nullptr);
	}

	VkCommandBuffer VulkanImageBufferObject::BeginSingleTimeCommands_(VkCommandPool& commandPool)
	{
		// create the temp command pool
		VkCommandPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createinfo.queueFamilyIndex = Renderer::getInstance()->GetQueueFamilyIndices().graphicsFamily;
		DebugLog::EC(vkCreateCommandPool(m_logicaldevice, &createinfo, nullptr, &commandPool));

		VkCommandBuffer commandbuffer = VK_NULL_HANDLE;

		// allocate the temporary command buffer
		VkCommandBufferAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocinfo.commandPool = commandPool;
		allocinfo.commandBufferCount = 1;
		allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &allocinfo, &commandbuffer));

		// immediately start recording this command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		DebugLog::EC(vkBeginCommandBuffer(commandbuffer, &beginInfo));

		return commandbuffer;
	}

	void VulkanImageBufferObject::EndSingleTimeCommands_(VkCommandBuffer commandBuffer, VkQueue queue)
	{
		DebugLog::EC(vkEndCommandBuffer(commandBuffer));

		// now execute the command buffer
		VkSubmitInfo submitinfo{};
		submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &commandBuffer;

		DebugLog::EC(vkQueueSubmit(queue, 1, &submitinfo, VK_NULL_HANDLE));
		DebugLog::EC(vkQueueWaitIdle(queue));
	}

	VkSampler VulkanImageBufferObject::CreateSampler_(bool mipmap, VkFilter filter, float miplevel, bool anisotropy, float anisotropylevel)
	{
		// image sampler creation
		VkSamplerCreateInfo samplerinfo{};
		samplerinfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerinfo.magFilter = filter;
		samplerinfo.minFilter = filter;
		samplerinfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerinfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerinfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerinfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerinfo.compareEnable = VK_FALSE;
		samplerinfo.compareOp = VK_COMPARE_OP_NEVER; // used for percentage-closer filtering on shadow maps
		samplerinfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

		if (mipmap)
		{
			samplerinfo.mipLodBias = 0.0f;
			samplerinfo.minLod = 0.0f;
			samplerinfo.maxLod = miplevel; // mipmapping lod
		}
		else
		{
			samplerinfo.mipLodBias = 0.0f;
			samplerinfo.minLod = 0.0f;
			samplerinfo.maxLod = 0.f; // mipmapping lod
		}

		if (anisotropy)
		{
			samplerinfo.maxAnisotropy = anisotropylevel; // check with gpu pls
			samplerinfo.anisotropyEnable = VK_TRUE;
		}
		else
		{
			samplerinfo.maxAnisotropy = 0.f; // check with gpu pls
			samplerinfo.anisotropyEnable = VK_FALSE;
		}

		VkSampler sampler = VK_NULL_HANDLE;
		DebugLog::EC(vkCreateSampler(m_logicaldevice, &samplerinfo, nullptr, &sampler));
		return sampler;
	}
}
