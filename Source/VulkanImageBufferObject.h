#ifndef VULKAN_IMAGE_BUFFER_OBJECT_H
#define VULKAN_IMAGE_BUFFER_OBJECT_H

#include "platform.h"
#include <vector>
#include <string>

namespace luna
{
	class VulkanImageBufferObject
	{
	public:
		VulkanImageBufferObject();
		virtual ~VulkanImageBufferObject();

		inline VkImage getImage() const { return m_image; }
		inline VkImageView getImageView() const { return m_imageview; }
		inline VkFormat getFormat() const { return m_format; }
		inline VkSampler getSampler() const { return m_sampler; }
		inline uint32_t getWidth() const { return m_texwidth; }
		inline uint32_t getHeight() const { return m_texheight; }

		/* get the sampler from the implementor (e.g attachments shared sampler with each other) */
		inline void setSampler(VkSampler sampler) { this->m_sampler = sampler; }
		static void TransitionImageLayout_(const VkCommandBuffer& buffer, VkImage srcimage, const VkImageLayout& oldLayout, const VkImageLayout& newLayout, const VkImageSubresourceRange& subresourceRange);
		static void TransitionAttachmentImagesLayout_(
			const VkCommandBuffer & commandbuffer, const VkImage& image,
			const VkImageLayout& oldlayout, const VkImageLayout& newlayout,
			VkAccessFlags srcaccessflag, VkAccessFlags dstaccessflag,
			VkPipelineStageFlags srcpipelinestage, VkPipelineStageFlags dstpipelinestage,
			VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
		);
		static VkCommandBuffer BeginSingleTimeCommands_(VkCommandPool& commandPool);
		static void EndSingleTimeCommands_(VkCommandBuffer commandBuffer, VkQueue queue);

	protected:
		static VkDevice m_logicaldevice;

		static void CreateStagingBuffer_(const VkDeviceSize& size, VkBuffer& stagingbuffer, VkDeviceMemory& stagingmemory, VkMemoryRequirements& memReqs);
		static void CopyBufferToImageBuffer_(const std::vector<VkBufferImageCopy>& bufferCopyRegions, 
			const VkImage& srcImage, VkBuffer& stagingbuffer, VkDeviceMemory& stagingmemory, const VkImageSubresourceRange& subresourceRange);
		static VkSampler CreateSampler_(bool mipmap = false, VkFilter filter = VK_FILTER_LINEAR, float miplevel = 0.0f, bool anisotropy = false, float anisotropylevel = 0.0f);

		VkFormat m_format = VK_FORMAT_UNDEFINED;
		size_t m_texsize = 0;
		uint32_t m_texwidth = 0;
		uint32_t m_texheight = 0;
		uint32_t m_mipLevels = 0;
		VkImage m_image = VK_NULL_HANDLE;
		VkImageView m_imageview = VK_NULL_HANDLE;
		VkSampler m_sampler = VK_NULL_HANDLE;
	};
}

#endif

