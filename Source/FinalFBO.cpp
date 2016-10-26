#include "FinalFBO.h"
#include "DebugLog.h"
#include "TextureResources.h"
#include "VulkanTexture2D.h"
#include "enum_c.h"

#include <array>

namespace luna
{
	std::once_flag FinalFBO::m_sflag{};
	VkRenderPass FinalFBO::m_renderpass = VK_NULL_HANDLE;

	FinalFBO::FinalFBO()
	{
		m_attachments.resize(FINAL_FBOATTs::ALL_ATTACHMENTS);
		m_clearvalues.resize(FINAL_FBOATTs::ALL_ATTACHMENTS);
	}

	FinalFBO::~FinalFBO()
	{
	}

	void FinalFBO::Init(const VkExtent2D & extent)
	{
		/* check valid extent mah */
		if (extent.height <= 0 || extent.width <= 0)
		{
			DebugLog::throwEx("extent invalid values for base fbo");
			return;
		}

		m_resolution = extent;

		CreateRenderPass_();

		VkImageView imageviews[] = {m_attachments[FINAL_FBOATTs::COLOR_ATTACHMENT].view};

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
		m_renderPassInfo.clearValueCount = static_cast<uint32_t>(m_clearvalues.size());
		m_renderPassInfo.pClearValues = m_clearvalues.data();
	}

	void FinalFBO::Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent)
	{
		// transition the image layout for the swapchain image
		VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
			commandbuffer, m_attachments[FINAL_FBOATTs::COLOR_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
			VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
		);

		vkCmdBeginRenderPass(commandbuffer, &m_renderPassInfo, subpasscontent);
	}

	void FinalFBO::CreateAttachments_()
	{
		TextureResources* texrsc = TextureResources::getInstance();
		VulkanImageBufferObject** imageattachment = nullptr; // image attachment mapping

		// ldr output color attachment
		imageattachment = &texrsc->Textures[eTEXTURES::LDRTEX_ATTACHMENT_RGBA8];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment(*imageattachment, FINAL_FBOATTs::COLOR_ATTACHMENT);
	}

	void FinalFBO::CreateRenderPass_()
	{
		auto lambda = [&]() {

			CreateAttachments_();

			VkAttachmentDescription attachmentDesc{};

			{
				attachmentDesc.format = m_attachments[FINAL_FBOATTs::COLOR_ATTACHMENT].format;
				attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store this image
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // images to be used in other shaders
			}

			VkSubpassDescription subPass{};
			VkAttachmentReference colorAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

			{
				subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subPass.colorAttachmentCount = 1;
				subPass.pColorAttachments = &colorAttachmentRef;
			}

			VkSubpassDependency dependency{};

			{
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0; // index 0 refer to our subPass .. which is the first and only one
				dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			}

			VkRenderPassCreateInfo renderpass_create_info{};
			renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderpass_create_info.attachmentCount = 1;
			renderpass_create_info.pAttachments = &attachmentDesc;
			renderpass_create_info.subpassCount = 1;
			renderpass_create_info.pSubpasses = &subPass;
			renderpass_create_info.dependencyCount = 1;
			renderpass_create_info.pDependencies = &dependency;

			DebugLog::EC(vkCreateRenderPass(m_logicaldevice, &renderpass_create_info, nullptr, &m_renderpass));
		};

		std::call_once(m_sflag, lambda);

		if (m_renderpass == VK_NULL_HANDLE)
		{
			DebugLog::throwEx("render pass is not available");
			return;
		}
	}

	void FinalFBO::Destroy()
	{
		// only fbo destroy here
		if (m_framebuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(m_logicaldevice, m_framebuffer, nullptr);
			m_framebuffer = VK_NULL_HANDLE;
		}

		if (m_renderpass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(m_logicaldevice, m_renderpass, nullptr);
			m_renderpass = VK_NULL_HANDLE;
		}

		// image views are from swap chain
		m_attachments.clear();
	}
}
