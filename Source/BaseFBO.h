#ifndef BASE_FBO_H
#define BASE_FBO_H

#include "Framebuffer.h"

namespace luna
{
	class BaseFBO :
		public Framebuffer
	{
	public:
		enum E_ATTACHMENTS
		{
			COLOR_ATTACHMENT = 0,
			ALL_ATTACHMENTS
		};

		BaseFBO();
		virtual ~BaseFBO();

		void Init(const VkExtent2D& extent) override;
		void Destroy() override;

		/* get the image view from the swap chain */
		void AddColorAttachment(const VkImageView& view, const VkFormat& format);

		/* get the render pass from renderer */
		void AddRenderPass(const VkRenderPass& renderpass);
	};
}

#endif

