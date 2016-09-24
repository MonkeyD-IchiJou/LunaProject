#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "platform.h"
#include <vector>

namespace luna
{
	struct FramebufferAttachment
	{
		VkImageView view = VK_NULL_HANDLE;
		VkFormat format = VK_FORMAT_UNDEFINED;

		FramebufferAttachment() {}
		~FramebufferAttachment() {}
	};

	class Framebuffer
	{
	public:
		Framebuffer();
		virtual ~Framebuffer();

		/* init the framebuffer */
		virtual void Init(const VkExtent2D& extent) = 0;

		/* destroy the framebuffer */
		virtual void Destroy() = 0;

		/* bind the framebuffer and begin the renderpass */
		virtual void Bind(const VkCommandBuffer& commandbuffer, VkSubpassContents subpasscontent = VK_SUBPASS_CONTENTS_INLINE);

		/* unbind the framebuffer and end the render pass */
		virtual void UnBind(const VkCommandBuffer& commandbuffer);

		/* get the render pass for this framebuffer */
		inline VkRenderPass getRenderPass() const { return m_renderpass; }

		/* get the width and height of the framebuffer */
		inline VkExtent2D getResolution() const { return m_resolution; }

		/* set the width and height of the framebuffer */
		virtual void setResolution(const VkExtent2D& extent);

	protected:
		/* framebuffer object itself */
		VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

		/* every framebuffer must have a renderpass to associates with */
		VkRenderPass m_renderpass = VK_NULL_HANDLE;

		/* every framebuffer must have at least one attachment to draw/read at*/
		std::vector<FramebufferAttachment> m_attachments;

		/* the resolution for this framebuffer */
		VkExtent2D m_resolution{};

		/* logical device is needed */
		VkDevice m_logicaldevice = VK_NULL_HANDLE;
	};
}

#endif

