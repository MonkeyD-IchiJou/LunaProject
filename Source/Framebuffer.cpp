#include "Framebuffer.h"
#include <array>
#include "Renderer.h"

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
}