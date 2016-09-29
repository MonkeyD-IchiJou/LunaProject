#include "BaseFBO.h"
#include "DebugLog.h"
#include <array>

namespace luna
{

	BaseFBO::BaseFBO()
	{
		m_attachments.resize(ALL_ATTACHMENTS);
	}

	BaseFBO::~BaseFBO()
	{
	}

	void BaseFBO::Init(const VkExtent2D & extent)
	{
		/* check valid extent mah */
		if (extent.height <= 0 || extent.width <= 0)
		{
			DebugLog::throwEx("extent invalid values for base fbo");
			return;
		}

		m_resolution = extent;

		// get the image views from swap chain pls
		if (m_attachments.size() == 0)
		{
			DebugLog::throwEx("no attachments available");
			return;
		}

		VkImageView imageviews[] = {m_attachments[COLOR_ATTACHMENT].view};

		// get the render pass from renderer pls
		if (m_renderpass == VK_NULL_HANDLE)
		{
			DebugLog::throwEx("render pass is not available");
			return;
		}

		// create the framebuffer
		VkFramebufferCreateInfo framebuffer_createinfo{};
		framebuffer_createinfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_createinfo.renderPass		= m_renderpass;
		framebuffer_createinfo.attachmentCount	= (uint32_t)m_attachments.size();
		framebuffer_createinfo.pAttachments		= imageviews;
		framebuffer_createinfo.width			= m_resolution.width;
		framebuffer_createinfo.height			= m_resolution.height;
		framebuffer_createinfo.layers			= 1; // vr related
		DebugLog::EC(vkCreateFramebuffer(m_logicaldevice, &framebuffer_createinfo, nullptr, &m_framebuffer));
	}

	void BaseFBO::AddColorAttachment(const VkImageView & view, const VkFormat & format)
	{
		FramebufferAttachment att{};
		att.format = format;
		att.view = view;

		m_attachments[COLOR_ATTACHMENT] = att;
	}

	void BaseFBO::AddRenderPass(const VkRenderPass & renderpass)
	{
		this->m_renderpass = renderpass;
	}

	void BaseFBO::Destroy()
	{
		// only fbo destroy here
		if (m_framebuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(m_logicaldevice, m_framebuffer, nullptr);
			m_framebuffer = VK_NULL_HANDLE;
		}

		// image views are from swap chain
		m_attachments.clear();
	}
}
