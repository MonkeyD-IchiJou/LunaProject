#ifndef PRESENTATION_FBO_H
#define PRESENTATION_FBO_H

#include "Framebuffer.h"
#include <mutex>

namespace luna
{
	namespace PRESENT_FBOATTs
	{
		enum eATTS
		{
			COLOR_ATTACHMENT = 0,
			ALL_ATTACHMENTS
		};
	}

	class PresentationFBO :
		public Framebuffer
	{
	public:
		PresentationFBO();
		virtual ~PresentationFBO();

		void Init(const VkExtent2D& extent) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent = VK_SUBPASS_CONTENTS_INLINE) override;

		static inline VkRenderPass getRenderPass() { return m_renderpass; }

	protected:
		void CreateRenderPass_() override;

	private:
		/* make sure only one renderpass throughout the whole fbo */
		static std::once_flag m_sflag;
		/* every framebuffer must have only renderpass to associates with */
		static VkRenderPass m_renderpass;
	};
}

#endif

