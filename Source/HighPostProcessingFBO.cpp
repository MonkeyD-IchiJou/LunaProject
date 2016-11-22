#include "HighPostProcessingFBO.h"
#include "DebugLog.h"
#include <array>
#include "TextureResources.h"
#include "VulkanTexture2D.h"
#include "enum_c.h"

namespace luna
{
	std::once_flag HighPostProcessingFBO::m_sflag{};
	VkRenderPass HighPostProcessingFBO::m_renderpass = VK_NULL_HANDLE;

	HighPostProcessingFBO::HighPostProcessingFBO()
	{
		m_attachments.resize(HPP_FBOATTs::ALL_ATTACHMENTS);
		m_clearvalues.resize(HPP_FBOATTs::ALL_ATTACHMENTS);
	}

	HighPostProcessingFBO::~HighPostProcessingFBO()
	{
		if (m_renderpass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(m_logicaldevice, m_renderpass, nullptr);
			m_renderpass = VK_NULL_HANDLE;
		}
	}

	void HighPostProcessingFBO::Init(const VkExtent2D & extent)
	{
		/* check valid extent mah */
		if (extent.height <= 0 || extent.width <= 0)
		{
			DebugLog::throwEx("extent invalid values for base fbo");
			return;
		}

		m_resolution = extent;

		// must create the render pass and own attachments
		CreateRenderPass_();

		VkImageView imageviews[] = { 
			m_attachments[HPP_FBOATTs::HDRCOLOR_ATTACHMENT].view
		};

		// create the framebuffer
		VkFramebufferCreateInfo framebuffer_createinfo{};
		framebuffer_createinfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_createinfo.renderPass		= m_renderpass;
		framebuffer_createinfo.attachmentCount	= static_cast<uint32_t>(m_attachments.size());
		framebuffer_createinfo.pAttachments		= imageviews;
		framebuffer_createinfo.width			= m_resolution.width;
		framebuffer_createinfo.height			= m_resolution.height;
		framebuffer_createinfo.layers			= 1; // vr related
		DebugLog::EC(vkCreateFramebuffer(m_logicaldevice, &framebuffer_createinfo, nullptr, &m_framebuffer));

		// init the renderpass info
		m_renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		m_renderPassInfo.renderPass = m_renderpass;
		m_renderPassInfo.framebuffer = m_framebuffer;
		m_renderPassInfo.renderArea.offset = { 0, 0 };
		m_renderPassInfo.renderArea.extent = m_resolution;
		m_renderPassInfo.clearValueCount = static_cast<uint32_t>(m_clearvalues.size());
		m_renderPassInfo.pClearValues = m_clearvalues.data();
	}

	void HighPostProcessingFBO::Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent)
	{
		// must make sure the images are optimal layout
		VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
			commandbuffer, m_attachments[HPP_FBOATTs::HDRCOLOR_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
			0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
		);

		// starting a render pass
		vkCmdBeginRenderPass(commandbuffer, &m_renderPassInfo, subpasscontent);
	}

	void HighPostProcessingFBO::CreateAttachments_()
	{
		TextureResources* texrsc = TextureResources::getInstance();
		VulkanImageBufferObject** imageattachment = nullptr; // image attachment mapping

		// final output color
		imageattachment = &texrsc->Textures[eTEXTURES::HPP_ATTACHMENT_RGBA16F];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_FILTER_NEAREST
		);
		SetAttachment(*imageattachment, HPP_FBOATTs::HDRCOLOR_ATTACHMENT);
	}

	void HighPostProcessingFBO::CreateRenderPass_()
	{
		auto lambda = [&]() {
		
			// create all the attachments first
			CreateAttachments_();
			
			// Describe all the attachments (how it is going to use in the renderpass)
			std::array<VkAttachmentDescription, HPP_FBOATTs::ALL_ATTACHMENTS> attachmentDescs = {};

			{
				// final output attachment description
				auto* descs = &attachmentDescs[HPP_FBOATTs::HDRCOLOR_ATTACHMENT];
				descs->format = m_attachments[HPP_FBOATTs::HDRCOLOR_ATTACHMENT].format;
				descs->samples = VK_SAMPLE_COUNT_1_BIT;
				descs->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store this attachment
				descs->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				descs->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				descs->initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				descs->finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // for shader input
			}

			// attachment ref preparation
			VkAttachmentReference final_outputAttachmentRef{HPP_FBOATTs::HDRCOLOR_ATTACHMENT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

			// all the subpasses
			std::array<VkSubpassDescription, HPP_FBOATTs::ALL_SUBPASS> subPass{};

			// subpass 0 - motion blur post process pass
			subPass[HPP_FBOATTs::eSUBPASS_MOTIONBLUR].flags = 0;
			subPass[HPP_FBOATTs::eSUBPASS_MOTIONBLUR].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subPass[HPP_FBOATTs::eSUBPASS_MOTIONBLUR].colorAttachmentCount = 1;
			subPass[HPP_FBOATTs::eSUBPASS_MOTIONBLUR].pColorAttachments = &final_outputAttachmentRef;

			// Subpass dependencies for layout transitions
			std::array<VkSubpassDependency, 2> dependencies{};

			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = HPP_FBOATTs::eSUBPASS_MOTIONBLUR;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = HPP_FBOATTs::eSUBPASS_MOTIONBLUR;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			// Render pass create
			VkRenderPassCreateInfo renderpass_create_info{};
			renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderpass_create_info.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
			renderpass_create_info.pAttachments = attachmentDescs.data();
			renderpass_create_info.subpassCount = static_cast<uint32_t>(subPass.size());
			renderpass_create_info.pSubpasses = subPass.data();
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

	void HighPostProcessingFBO::Destroy()
	{
		// only fbo destroy here
		if (m_framebuffer != VK_NULL_HANDLE)
		{
			vkDestroyFramebuffer(m_logicaldevice, m_framebuffer, nullptr);
			m_framebuffer = VK_NULL_HANDLE;
		}

		// image views are from swap chain
		m_attachments.clear();
	}
}