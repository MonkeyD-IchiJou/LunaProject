#include "Framebuffer.h"
#include "Renderer.h"
#include "VulkanTexture2D.h"

namespace luna
{
	Framebuffer::Framebuffer()
	{
		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();
	}

	Framebuffer::~Framebuffer()
	{
	}

	void Framebuffer::UnBind(const VkCommandBuffer & commandbuffer)
	{
		vkCmdEndRenderPass(commandbuffer);
	}

	void Framebuffer::setResolution(const VkExtent2D & extent)
	{
	}

	void Framebuffer::SetAttachment(const VkImage & image, const VkImageView & view, const VkFormat & format, const uint32_t & i)
	{
		FramebufferAttachment att{};
		att.image = image;
		att.format = format;
		att.view = view;

		m_attachments[i] = att;
	}

	void Framebuffer::SetAttachment(const VulkanImageBufferObject * image, const uint32_t & i)
	{
		FramebufferAttachment attach{};
		attach.image = image->getImage();
		attach.format = image->getFormat();
		attach.view = image->getImageView();
		m_attachments[i] = attach;
	}
}