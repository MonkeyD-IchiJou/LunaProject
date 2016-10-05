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

		inline auto getImage() const { return m_image; }
		inline auto getImageView() const { return m_imageview; }
		inline auto getFormat() const { return m_format; }
		inline auto getSampler() const { return m_sampler; }

		static void TransitionImageLayout_(const VkCommandBuffer& buffer, VkImage srcimage, const VkImageLayout& oldLayout, const VkImageLayout& newLayout, const VkImageSubresourceRange& subresourceRange);

	protected:
		static VkDevice m_logicaldevice;

		static void CreateStagingBuffer_(const VkDeviceSize& size, VkBuffer& stagingbuffer, VkDeviceMemory& stagingmemory, VkMemoryRequirements& memReqs);
		static void CopyBufferToImageBuffer_(const std::vector<VkBufferImageCopy>& bufferCopyRegions, 
			const VkImage& srcImage, VkBuffer& stagingbuffer, VkDeviceMemory& stagingmemory, const VkImageSubresourceRange& subresourceRange);
		static VkCommandBuffer BeginSingleTimeCommands_(VkCommandPool& commandPool);
		static void EndSingleTimeCommands_(VkCommandBuffer commandBuffer, VkQueue queue);
		void CreateSampler(bool mipmap = true, bool anisotrophy = true);

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

