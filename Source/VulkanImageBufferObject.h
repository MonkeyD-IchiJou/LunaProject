#ifndef VULKAN_IMAGE_BUFFER_OBJECT_H
#define VULKAN_IMAGE_BUFFER_OBJECT_H

#include "VulkanBufferData.h"

namespace luna
{
	class VulkanImageBufferObject
	{
	public:
		VulkanImageBufferObject();
		virtual ~VulkanImageBufferObject();

		/* create the image view */
		virtual void CreateImageView(const VkFormat& format, const VkImageAspectFlags& aspectFlags) = 0;

		/* get the main buffer data */
		inline const auto& GetMainBufferData() const { return m_imagebuff; }

		/* get the image view handler */
		inline const auto GetImageView() const { return m_imageview; }

		/* get the buffer offset in the main device memory */
		inline const auto GetBufferOffset() const { return m_imageBufferOffset; }

		/* set the image buffer offset in the device memory */
		inline void SetImageBufferOffset(VkDeviceSize imageBufferOffset) { this->m_imageBufferOffset = imageBufferOffset; }

	protected:
		/* create the image buffer and query memory requirement info for it */
		void CreateImageBuff_(VulkanImageBufferData& buff, const VkMemoryPropertyFlags & memoryproperties);

		/* image layout transitioning for better usage in gpu */
		void TransitionImageLayout_(VkImage srcimage, const VkImageLayout& oldLayout, const VkImageLayout& newLayout, const VkImageAspectFlags& aspectMask, 
			const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask);
		static VkCommandBuffer BeginSingleTimeCommands_(const VkDevice & logicaldevice, VkCommandPool commandPool);
		static void EndSingleTimeCommands_(const VkDevice & logicaldevice, VkCommandBuffer commandBuffer);

	protected:
		/* the total width of this image */
		int m_texwidth = 0;

		/* the total height of this image */
		int m_texheight = 0;

		/* total image size in bytes */
		VkDeviceSize m_imageTotalSize = 0;

		/* the beginning offset (aligned bytes) of the image buffer to be mapped of, in the device memory */
		VkDeviceSize m_imageBufferOffset = 0;

		/* the main image buffer to communicate with when drawing */
		VulkanImageBufferData m_imagebuff{};

		/* handle image view */
		VkImageView m_imageview = VK_NULL_HANDLE;

		/* handle for the logical device */
		VkDevice m_logicaldevice = VK_NULL_HANDLE;
	};
}

#endif

