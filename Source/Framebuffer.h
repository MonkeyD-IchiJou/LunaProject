#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "platform.h"
#include <vector>


namespace luna
{
	struct FramebufferAttachment
	{
		VkImage image = VK_NULL_HANDLE;
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
		virtual void Bind(const VkCommandBuffer& commandbuffer, VkSubpassContents subpasscontent = VK_SUBPASS_CONTENTS_INLINE) = 0;

		/* unbind the framebuffer and end the render pass */
		void UnBind(const VkCommandBuffer& commandbuffer);

		/* get the width and height of the framebuffer */
		inline VkExtent2D getResolution() const { return m_resolution; }

		/* set the width and height of the framebuffer */
		virtual void setResolution(const VkExtent2D& extent);

	protected:
		/* must create its own unique render pass */
		virtual void CreateRenderPass_() = 0;

		/* attachment images layout transition before binding fbo/using it */
		virtual void TransitionAttachmentImagesLayout_(const VkCommandBuffer & commandbuffer) = 0;

		/* framebuffer object itself */
		VkFramebuffer m_framebuffer = VK_NULL_HANDLE;

		/* every framebuffer must have at least one attachment to draw/read at*/
		std::vector<FramebufferAttachment> m_attachments;

		/* clear color/depth at the beggining of the frame */
		std::vector<VkClearValue> m_clearvalues;

		/* the resolution for this framebuffer */
		VkExtent2D m_resolution{};

		/* logical device is needed */
		VkDevice m_logicaldevice = VK_NULL_HANDLE;
	};
}

#endif

