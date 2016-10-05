#ifndef MRT_FBO_H
#define MRT_FBO_H

#include "Framebuffer.h"
#include <mutex>

namespace luna
{
	class VulkanImageBufferObject;

	class MrtFBO :
		public Framebuffer
	{
	public:
		enum E_ATTACHMENTS
		{
			WORLDPOS_ATTACHMENT = 0,
			WORLDNORM_ATTACHMENT,
			ALBEDO_ATTACHMENT,
			DEPTH_ATTACHMENT,
			ALL_ATTACHMENTS
		};

		MrtFBO();
		virtual ~MrtFBO();

		void Init(const VkExtent2D& extent) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent = VK_SUBPASS_CONTENTS_INLINE) override;

		void ClearColor(const VkClearColorValue& worldpos_clearvalue, const VkClearColorValue& worldnorm_clearvalue, const VkClearColorValue& albedo_clearvalue);
		void ClearDepthStencil(VkClearDepthStencilValue& cleardepthstencil);

		static inline VkRenderPass getRenderPass() { return m_renderpass; }

	protected:
		void CreateRenderPass_() override;
		void SetAttachment_(const VulkanImageBufferObject* image, E_ATTACHMENTS att);
		/* make sure the attachment input layout is color optimal */
		void TransitionAttachmentImagesLayout_(const VkCommandBuffer & commandbuffer) override;

	private:
		/* make sure only one renderpass throughout the whole fbo */
		static std::once_flag m_sflag;
		/* every framebuffer must have only one renderpass to associates with */
		static VkRenderPass m_renderpass;
	};

}

#endif

