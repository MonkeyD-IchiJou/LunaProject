#ifndef BASIC_IMAGE_ATTACHMENT_H
#define BASIC_IMAGE_ATTACHMENT_H

#include "VulkanImageBufferObject.h"

namespace luna
{
	class BasicImageAttachment :
		public VulkanImageBufferObject
	{
	public:
		BasicImageAttachment(const uint32_t& texwidth, const uint32_t& texheight, const VkImageAspectFlags & aspectFlags);
		virtual ~BasicImageAttachment();

		/* create the image buffer */
		void CreateImageBuffer(const VkImageTiling& tiling, const VkFormat& format, const VkImageUsageFlags& usage);

		/* create the image view */
		void CreateImageView(const VkFormat& format, const VkImageAspectFlags& aspectFlags) override;

		/* create the image view dynamically based on the image format */
		void CreateImageView();

	private:
		VkImageAspectFlags m_aspectFlags = VK_NULL_HANDLE;

	};
}

#endif

