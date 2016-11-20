#ifndef MRT_FBO_H
#define MRT_FBO_H

#include "Framebuffer.h"
#include <mutex>

namespace luna
{
	class VulkanImageBufferObject;

	namespace DFR_FBOATTs
	{
		enum eATTS
		{
			COLOR0_ATTACHMENT = 0,
			COLOR1_ATTACHMENT,
			COLOR2_ATTACHMENT,
			LIGHTINGCOLOR_ATTACHMENT,
			NONLIGHTINGCOLOR_ATTACHMENT,
			HDRCOLOR_ATTACHMENT,
			DEPTH32F_ATTACHMENT,
			ALL_ATTACHMENTS
		};

		enum eSUBPASS
		{
			eSUBPASS_GBUFFER = 0,
			eSUBPASS_LIGHTING,
			eSUBPASS_NONLIGHTING,
			eSUBPASS_COMPOSITE,
			ALL_SUBPASS
		};
	}

	class DeferredFBO :
		public Framebuffer
	{
	public:
		DeferredFBO();
		virtual ~DeferredFBO();

		void Init(const VkExtent2D& extent) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent = VK_SUBPASS_CONTENTS_INLINE) override;

		static inline VkRenderPass getRenderPass() { return m_renderpass; }

	protected:
		void CreateAttachments_();
		void CreateRenderPass_() override;

	private:
		/* make sure only one renderpass throughout the whole fbo */
		static std::once_flag m_sflag;
		/* every framebuffer must have only one renderpass to associates with */
		static VkRenderPass m_renderpass;
	};

}

#endif

