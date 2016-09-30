#include "BasicImageAttachment.h"
#include "Renderer.h"
#include "DebugLog.h"

namespace luna
{

	BasicImageAttachment::BasicImageAttachment(const uint32_t& texwidth, const uint32_t& texheight, const VkImageAspectFlags & aspectFlags)
	{
		m_texwidth = texwidth;
		m_texheight = texheight;
		m_aspectFlags = aspectFlags;

		m_imageTotalSize = m_texwidth * m_texheight * 4; // rgba
	}

	BasicImageAttachment::~BasicImageAttachment()
	{
	}

	void BasicImageAttachment::CreateImageBuffer(const VkImageTiling & tiling, const VkFormat & format, const VkImageUsageFlags & usage)
	{
		// create the main image buff 
		m_imagebuff.format = format;
		m_imagebuff.tiling = tiling;
		m_imagebuff.imageusage = usage;
		CreateImageBuff_(m_imagebuff, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	void BasicImageAttachment::CreateImageView(const VkFormat & format, const VkImageAspectFlags & aspectFlags)
	{
	}

	void BasicImageAttachment::CreateImageView()
	{
		// transition the layout based on what kind of image is this
		switch (m_aspectFlags)
		{
		case VK_IMAGE_ASPECT_COLOR_BIT:

			break;

		case VK_IMAGE_ASPECT_DEPTH_BIT:
			TransitionImageLayout_(m_imagebuff.buffer,
				VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
				m_aspectFlags,
				0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT
			);
			break;

		case VK_IMAGE_ASPECT_STENCIL_BIT:

			break;

		default:
			DebugLog::throwEx("aspect flags for image attachment not available");
			break;
		}

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_imagebuff.buffer;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = m_imagebuff.format;
		viewInfo.subresourceRange.aspectMask = m_aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		DebugLog::EC(vkCreateImageView(m_logicaldevice, &viewInfo, nullptr, &m_imageview));
	}

}
