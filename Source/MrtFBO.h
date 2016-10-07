#ifndef MRT_FBO_H
#define MRT_FBO_H

#include "Framebuffer.h"
#include <mutex>

namespace luna
{
	class VulkanImageBufferObject;

	namespace MRT_FBOATTs
	{
		enum eATTS
		{
			WORLDPOS_ATTACHMENT = 0,
			WORLDNORM_ATTACHMENT,
			ALBEDO_ATTACHMENT,
			DEPTH_ATTACHMENT,
			ALL_ATTACHMENTS
		};
	}

	class MrtFBO :
		public Framebuffer
	{
	public:
		MrtFBO();
		virtual ~MrtFBO();

		void Init(const VkExtent2D& extent) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent = VK_SUBPASS_CONTENTS_INLINE) override;

		static inline VkRenderPass getRenderPass() { return m_renderpass; }

	protected:
		void CreateRenderPass_() override;
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

