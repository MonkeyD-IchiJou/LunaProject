#include "DeferredFBO.h"
#include "DebugLog.h"
#include <array>
#include "TextureResources.h"
#include "VulkanTexture2D.h"
#include "enum_c.h"

namespace luna
{
	std::once_flag DeferredFBO::m_sflag{};
	VkRenderPass DeferredFBO::m_renderpass = VK_NULL_HANDLE;

	DeferredFBO::DeferredFBO()
	{
		m_attachments.resize(DFR_FBOATTs::ALL_ATTACHMENTS);
		m_clearvalues.resize(DFR_FBOATTs::ALL_ATTACHMENTS);
	}

	DeferredFBO::~DeferredFBO()
	{
	}

	void DeferredFBO::Init(const VkExtent2D & extent)
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
			m_attachments[DFR_FBOATTs::WORLDPOS_ATTACHMENT].view, 
			m_attachments[DFR_FBOATTs::WORLDNORM_ATTACHMENT].view, 
			m_attachments[DFR_FBOATTs::ALBEDO_ATTACHMENT].view,
			m_attachments[DFR_FBOATTs::HDRCOLOR_ATTACHMENT].view,
			m_attachments[DFR_FBOATTs::DEPTHSTENCIL_ATTACHMENT].view
		};

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

		// init the renderpass info
		m_renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		m_renderPassInfo.renderPass = m_renderpass;
		m_renderPassInfo.framebuffer = m_framebuffer;
		m_renderPassInfo.renderArea.offset = { 0, 0 };
		m_renderPassInfo.renderArea.extent = m_resolution;
		m_renderPassInfo.clearValueCount = static_cast<uint32_t>(m_clearvalues.size());
		m_renderPassInfo.pClearValues = m_clearvalues.data();
	}

	void DeferredFBO::Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent)
	{
		// must make sure the images are optimal layout
		VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
			commandbuffer, m_attachments[DFR_FBOATTs::WORLDPOS_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
			0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
		);

		VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
			commandbuffer, m_attachments[DFR_FBOATTs::WORLDNORM_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
			0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
		);

		VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
			commandbuffer, m_attachments[DFR_FBOATTs::ALBEDO_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
			0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
		);

		VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
			commandbuffer, m_attachments[DFR_FBOATTs::HDRCOLOR_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
			0, VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
		);

		// starting a render pass
		vkCmdBeginRenderPass(commandbuffer, &m_renderPassInfo, subpasscontent);
	}

	void DeferredFBO::CreateAttachments_()
	{
		TextureResources* texrsc = TextureResources::getInstance();
		VulkanImageBufferObject** imageattachment = nullptr; // image attachment mapping

		 // world pos color attachment
		imageattachment = &texrsc->Textures[eTEXTURES::WORLDPOS_ATTACHMENT_RGBA16F];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, // will be used by subpasses
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment(*imageattachment, DFR_FBOATTs::WORLDPOS_ATTACHMENT);

		// world normal color attachment
		imageattachment = &texrsc->Textures[eTEXTURES::WORLDNORM_ATTACHMENT_RGBA16F];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_R16G16B16A16_SFLOAT, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, // will be used by subpasses
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment(*imageattachment, DFR_FBOATTs::WORLDNORM_ATTACHMENT); 

		// albedo color attachment
		imageattachment = &texrsc->Textures[eTEXTURES::ALBEDO_ATTACHMENT_RGBA16F];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_R16G16B16A16_SFLOAT, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT, // will be used by subpasses
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment(*imageattachment, DFR_FBOATTs::ALBEDO_ATTACHMENT);

		// HDR color attachment
		imageattachment = &texrsc->Textures[eTEXTURES::HDRTEX_ATTACHMENT_RGBA16F];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_R16G16B16A16_SFLOAT, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT, // output by subpass and will be used by compute shader
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment(*imageattachment, DFR_FBOATTs::HDRCOLOR_ATTACHMENT);

		// depth buffer
		imageattachment = &texrsc->Textures[eTEXTURES::DEPTHSTENCIL_ATTACHMENT_32F8U];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_D32_SFLOAT_S8_UINT, // mobile not supported
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		SetAttachment(*imageattachment, DFR_FBOATTs::DEPTHSTENCIL_ATTACHMENT); // depth stencil attachment
	}

	void DeferredFBO::CreateRenderPass_()
	{
		// only called once
		auto lambda = [&]() {

			// create all the attachments first
			CreateAttachments_();

			// Describe all the attachments (how it is going to use in the renderpass)
			std::array<VkAttachmentDescription, DFR_FBOATTs::ALL_ATTACHMENTS> attachmentDescs = {};

			{
				// world position attachment description
				auto* descs = &attachmentDescs[DFR_FBOATTs::WORLDPOS_ATTACHMENT];
				descs->format = m_attachments[DFR_FBOATTs::WORLDPOS_ATTACHMENT].format;
				descs->samples = VK_SAMPLE_COUNT_1_BIT;
				descs->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // dun care about storing this attachment
				descs->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				descs->initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				descs->finalLayout = VK_IMAGE_LAYOUT_GENERAL; // for second subpass to input it

				// world normal attachment description
				descs = &attachmentDescs[DFR_FBOATTs::WORLDNORM_ATTACHMENT];
				descs->format = m_attachments[DFR_FBOATTs::WORLDNORM_ATTACHMENT].format;
				descs->samples = VK_SAMPLE_COUNT_1_BIT;
				descs->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // dun care about storing this attachment
				descs->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				descs->initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				descs->finalLayout = VK_IMAGE_LAYOUT_GENERAL; // for second subpass to input it

				// albedo attachment description
				descs = &attachmentDescs[DFR_FBOATTs::ALBEDO_ATTACHMENT];
				descs->format = m_attachments[DFR_FBOATTs::ALBEDO_ATTACHMENT].format;
				descs->samples = VK_SAMPLE_COUNT_1_BIT;
				descs->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // dun care about storing this attachment
				descs->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				descs->initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				descs->finalLayout = VK_IMAGE_LAYOUT_GENERAL; // for second subpass to input it

				// hdr attachment description
				descs = &attachmentDescs[DFR_FBOATTs::HDRCOLOR_ATTACHMENT];
				descs->format = m_attachments[DFR_FBOATTs::HDRCOLOR_ATTACHMENT].format;
				descs->samples = VK_SAMPLE_COUNT_1_BIT;
				descs->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->storeOp = VK_ATTACHMENT_STORE_OP_STORE; // going to store this attachment, to be used to present later
				descs->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				descs->initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				descs->finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // for other shaders to access it

				// depth attachment description
				descs = &attachmentDescs[DFR_FBOATTs::DEPTHSTENCIL_ATTACHMENT];
				descs->format = m_attachments[DFR_FBOATTs::DEPTHSTENCIL_ATTACHMENT].format;
				descs->samples = VK_SAMPLE_COUNT_1_BIT;
				descs->loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // not going to store this depth/stencil buffer
				descs->stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				descs->stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				descs->initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
				descs->finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			}

			// i have 2 subpass
			std::array<VkSubpassDescription, 2> subPass{};

			std::array<VkAttachmentReference, 3> outputAttachmentRef = {};
			outputAttachmentRef[0] = { DFR_FBOATTs::WORLDPOS_ATTACHMENT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			outputAttachmentRef[1] = { DFR_FBOATTs::WORLDNORM_ATTACHMENT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
			outputAttachmentRef[2] = { DFR_FBOATTs::ALBEDO_ATTACHMENT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

			std::array<VkAttachmentReference, 3> inputAttachmentRef = {};
			inputAttachmentRef[0] = { DFR_FBOATTs::WORLDPOS_ATTACHMENT, VK_IMAGE_LAYOUT_GENERAL };
			inputAttachmentRef[1] = { DFR_FBOATTs::WORLDNORM_ATTACHMENT, VK_IMAGE_LAYOUT_GENERAL };
			inputAttachmentRef[2] = { DFR_FBOATTs::ALBEDO_ATTACHMENT, VK_IMAGE_LAYOUT_GENERAL };

			VkAttachmentReference depthstencilAttachmentRef = {DFR_FBOATTs::DEPTHSTENCIL_ATTACHMENT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
			VkAttachmentReference presentColorAttachmentRef{DFR_FBOATTs::HDRCOLOR_ATTACHMENT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

			{
				// first subpass
				// this subpass output the neccessary gbuffer
				subPass[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subPass[0].colorAttachmentCount = static_cast<uint32_t>(outputAttachmentRef.size());
				subPass[0].pColorAttachments = outputAttachmentRef.data();
				subPass[0].pDepthStencilAttachment = &depthstencilAttachmentRef;

				// second subpass
				// take in the 3 attachment and output 1 hdr attachment
				subPass[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subPass[1].colorAttachmentCount = 1;
				subPass[1].pColorAttachments = &presentColorAttachmentRef; // only got 1 color attachment, layout = 0 in shaders
				subPass[1].inputAttachmentCount = static_cast<uint32_t>(inputAttachmentRef.size());
				subPass[1].pInputAttachments = inputAttachmentRef.data();
				subPass[1].pDepthStencilAttachment = &depthstencilAttachmentRef;
			}

			// dependency between subpasses
			VkSubpassDependency dependency{};

			{
				dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
				dependency.srcSubpass = 0; // subpass 0 start first
				dependency.dstSubpass = 1; // subpass 1 start after srcsubpass 0
				dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;
				dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT; // beginning work here
				dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT; // must finish all the works before this stage (transitioning image layout e.g)
			}

			// Render pass create
			VkRenderPassCreateInfo renderpass_create_info{};
			renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderpass_create_info.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
			renderpass_create_info.pAttachments = attachmentDescs.data();
			renderpass_create_info.subpassCount = static_cast<uint32_t>(subPass.size());
			renderpass_create_info.pSubpasses = subPass.data();
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

	void DeferredFBO::Destroy()
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
