#ifndef HIGH_POST_PROCESSING_FBO_H
#define HIGH_POST_PROCESSING_FBO_H

#include "Framebuffer.h"
#include <mutex>

namespace luna
{
	class VulkanImageBufferObject;

	namespace HPP_FBOATTs
	{
		enum eATTS
		{
			HDRCOLOR_ATTACHMENT = 0,
			ALL_ATTACHMENTS
		};

		enum eSUBPASS
		{
			eSUBPASS_MOTIONBLUR = 0,
			ALL_SUBPASS
		};
	}

	class HighPostProcessingFBO :
		public Framebuffer
	{
	public:
		HighPostProcessingFBO();
		virtual ~HighPostProcessingFBO();

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

