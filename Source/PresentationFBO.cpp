#include "PresentationFBO.h"
#include "DebugLog.h"
#include <array>
#include "VulkanImageBufferObject.h"
#include "enum_c.h"

namespace luna
{
	std::once_flag PresentationFBO::m_sflag{};
	VkRenderPass PresentationFBO::m_renderpass = VK_NULL_HANDLE;

	PresentationFBO::PresentationFBO()
	{
		m_attachments.resize(PRESENT_FBOATTs::ALL_ATTACHMENTS);
		m_clearvalues.resize(PRESENT_FBOATTs::ALL_ATTACHMENTS);
	}

	PresentationFBO::~PresentationFBO()
	{
		if (m_renderpass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(m_logicaldevice, m_renderpass, nullptr);
			m_renderpass = VK_NULL_HANDLE;
		}
	}

	void PresentationFBO::Init(const VkExtent2D & extent)
	{
		/* check valid extent mah */
		if (extent.height <= 0 || extent.width <= 0)
		{
			DebugLog::throwEx("extent invalid values for base fbo");
			return;
		}

		m_resolution = extent;

		CreateRenderPass_();

		VkImageView imageviews[] = {m_attachments[PRESENT_FBOATTs::COLOR_ATTACHMENT].view};

		// create the framebuffer
		VkFramebufferCreateInfo framebuffer_createinfo{};
		framebuffer_createinfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_createinfo.renderPass		= m_renderpass;
		framebuffer_createinfo.attachmentCount	= (uint32_t)m_attachments.size();
		framebuffer_createinfo.pAttachments		= imageviews;
		framebuffer_createinfo.width			= m_resolution.width;
		framebuffer_createinfo.height			= m_resolution.height;
		framebuffer_createinfo.layers			= 1; // vr related
		DebugLog::EC(vkCreateFramebuffer(m_logicaldevice, &framebuffer_createinfo, nullptr, &m_framebuffer));

		// render pass info preinit
		m_renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		m_renderPassInfo.renderPass = m_renderpass;
		m_renderPassInfo.framebuffer = m_framebuffer;
		m_renderPassInfo.renderArea.offset = { 0, 0 };
		m_renderPassInfo.renderArea.extent = m_resolution;
	}

	void PresentationFBO::Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent)
	{
		// transition the image layout for the swapchain image
		VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
			commandbuffer, m_attachments[PRESENT_FBOATTs::COLOR_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
			0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
		);

		vkCmdBeginRenderPass(commandbuffer, &m_renderPassInfo, subpasscontent);
	}

	void PresentationFBO::CreateRenderPass_()
	{
		auto lambda = [&]() {

			VkAttachmentDescription attachmentDesc{};

			{
				attachmentDesc.format = m_attachments[PRESENT_FBOATTs::COLOR_ATTACHMENT].format;
				attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store this image so that i can present it on the screen
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // images to be presented in the swap chain
			}

			VkSubpassDescription subPass{};
			VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

			{
				subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subPass.colorAttachmentCount = 1;
				subPass.pColorAttachments = &colorAttachmentRef;
			}

			std::array<VkSubpassDependency, 2> dependencies;

			{
				dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
				dependencies[0].dstSubpass = 0; // index 0 refer to our subPass .. which is the first and only one
				dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

				dependencies[1].srcSubpass = 0;
				dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
				dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			}

			VkRenderPassCreateInfo renderpass_create_info{};
			renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderpass_create_info.attachmentCount = 1;
			renderpass_create_info.pAttachments = &attachmentDesc;
			renderpass_create_info.subpassCount = 1;
			renderpass_create_info.pSubpasses = &subPass;
			renderpass_create_info.dependencyCount = static_cast<uint32_t>(dependencies.size());
			renderpass_create_info.pDependencies = dependencies.data();

			DebugLog::EC(vkCreateRenderPass(m_logicaldevice, &renderpass_create_info, nullptr, &m_renderpass));
		};

		std::call_once(m_sflag, lambda);

		if (m_renderpass == VK_NULL_HANDLE)
		{
			DebugLog::throwEx("render pass is not available");
			return;
		}
	}

	void PresentationFBO::Destroy()
	{
		// only fbo destroy here
		if (m_framebuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(m_logicaldevice, m_framebuffer, nullptr);
			m_framebuffer = VK_NULL_HANDLE;
		}

		// image views are from swap chain
		m_attachments.clear();
		m_clearvalues.clear();

		m_attachments.resize(PRESENT_FBOATTs::ALL_ATTACHMENTS);
		m_clearvalues.resize(PRESENT_FBOATTs::ALL_ATTACHMENTS);
	}
}
