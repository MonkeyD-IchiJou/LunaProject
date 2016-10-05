#ifndef FINAL_FBO_H
#define FINAL_FBO_H

#include "Framebuffer.h"
#include <mutex>

namespace luna
{
	class FinalFBO :
		public Framebuffer
	{
	public:
		enum E_ATTACHMENTS
		{
			COLOR_ATTACHMENT = 0,
			ALL_ATTACHMENTS
		};

		FinalFBO();
		virtual ~FinalFBO();

		void Init(const VkExtent2D& extent) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent = VK_SUBPASS_CONTENTS_INLINE) override;

		/* get the image view from the swap chain */
		void SetColorAttachment(const VkImage& image, const VkImageView& view, const VkFormat& format);

		/* clear color value begining of the frame */
		void ClearColor(const VkClearColorValue& clearvalue);

		static inline VkRenderPass getRenderPass() { return m_renderpass; }

	protected:
		void CreateRenderPass_() override;
		void TransitionAttachmentImagesLayout_(const VkCommandBuffer & commandbuffer) override;

	private:
		/* make sure only one renderpass throughout the whole fbo */
		static std::once_flag m_sflag;
		/* every framebuffer must have only renderpass to associates with */
		static VkRenderPass m_renderpass;
	};
}

#endif

