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

	void Framebuffer::Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent)
	{
		// starting a render pass 
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.f, 1.f, 0.f, 1.0f };
		clearValues[1].depthStencil = { 1.f, 0 };

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType								= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass							= m_renderpass;
		renderPassInfo.framebuffer							= m_framebuffer;
		renderPassInfo.renderArea.offset					= { 0, 0 };
		renderPassInfo.renderArea.extent					= m_resolution;
		renderPassInfo.clearValueCount						= (uint32_t) clearValues.size();
		renderPassInfo.pClearValues							= clearValues.data();

		vkCmdBeginRenderPass(commandbuffer, &renderPassInfo, subpasscontent);
	}

	void Framebuffer::UnBind(const VkCommandBuffer & commandbuffer)
	{
		vkCmdEndRenderPass(commandbuffer);
	}

	void Framebuffer::setResolution(const VkExtent2D & extent)
	{
	}
}