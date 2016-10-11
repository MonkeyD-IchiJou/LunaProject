#ifndef TEST_FBO_H
#define TEST_FBO_H

#include "Framebuffer.h"
#include <mutex>

namespace luna
{
	namespace TEST_FBOATTs
	{
		enum eATTS
		{
			COLOR_ATTACHMENT = 0,
			PRESENT_ATTACHMENT,
			ALL_ATTACHMENTS
		};
	}

	class TestFBO :
		public Framebuffer
	{
	public:
		TestFBO();
		virtual ~TestFBO();

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

