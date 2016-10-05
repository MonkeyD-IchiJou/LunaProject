#include "MrtFBO.h"
#include "DebugLog.h"
#include <array>
#include "TextureResources.h"
#include "VulkanTexture2D.h"

namespace luna
{
	std::once_flag MrtFBO::m_sflag{};
	VkRenderPass MrtFBO::m_renderpass = VK_NULL_HANDLE;

	MrtFBO::MrtFBO()
	{
		m_attachments.resize(ALL_ATTACHMENTS);
		m_clearvalues.resize(ALL_ATTACHMENTS);
	}

	MrtFBO::~MrtFBO()
	{
	}

	void MrtFBO::Init(const VkExtent2D & extent)
	{
		/* check valid extent mah */
		if (extent.height <= 0 || extent.width <= 0)
		{
			DebugLog::throwEx("extent invalid values for base fbo");
			return;
		}

		m_resolution = extent;

		TextureResources* texrsc = TextureResources::getInstance();
		VulkanImageBufferObject** imageattachment = nullptr; // image attachment mapping

		imageattachment = &texrsc->Textures[eTEXTURES::WORLDPOS_2D_RGBA16FLOAT];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_R16G16B16A16_SFLOAT, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment_(*imageattachment, WORLDPOS_ATTACHMENT); // world pos color attachment

		imageattachment = &texrsc->Textures[eTEXTURES::WORLDNORMAL_2D_RGBA16FLOAT];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_R16G16B16A16_SFLOAT, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment_(*imageattachment, WORLDNORM_ATTACHMENT); // world normal color attachment

		imageattachment = &texrsc->Textures[eTEXTURES::ALBEDO_2D_RGBA8UNORM];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_R8G8B8A8_UNORM, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment_(*imageattachment, ALBEDO_ATTACHMENT); // albedo color attachment

		imageattachment = &texrsc->Textures[eTEXTURES::DEPTH_2D_32FLOAT];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, VK_FORMAT_D32_SFLOAT, 
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, 
			VK_IMAGE_ASPECT_DEPTH_BIT, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
		);
		SetAttachment_(*imageattachment, DEPTH_ATTACHMENT); // depth stencil attachment

		VkImageView imageviews[] = { 
			m_attachments[WORLDPOS_ATTACHMENT].view, 
			m_attachments[WORLDNORM_ATTACHMENT].view, 
			m_attachments[ALBEDO_ATTACHMENT].view, 
			m_attachments[DEPTH_ATTACHMENT].view
		};

		// must create the render pass 
		CreateRenderPass_();

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
	}

	void MrtFBO::Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent)
	{
		// must make sure the images are optimal layout
		TransitionAttachmentImagesLayout_(commandbuffer);

		// starting a render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass			= m_renderpass;
		renderPassInfo.framebuffer			= m_framebuffer;
		renderPassInfo.renderArea.offset	= { 0, 0 };
		renderPassInfo.renderArea.extent	= m_resolution;
		renderPassInfo.clearValueCount		= static_cast<uint32_t>(m_clearvalues.size());
		renderPassInfo.pClearValues			= m_clearvalues.data();

		vkCmdBeginRenderPass(commandbuffer, &renderPassInfo, subpasscontent);
	}

	void MrtFBO::TransitionAttachmentImagesLayout_(const VkCommandBuffer & commandbuffer)
	{
		VkImageSubresourceRange subresourceRange{};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; 
		subresourceRange.baseMipLevel = 0; // Start at first mip level
		subresourceRange.levelCount = 1; // We will transition on all mip levels
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1; // The 2D texture only has one layer

		// make sure the image layout is suitable for any usage later 
		VulkanImageBufferObject::TransitionImageLayout_(
			commandbuffer,
			m_attachments[WORLDPOS_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			subresourceRange
		);

		VulkanImageBufferObject::TransitionImageLayout_(
			commandbuffer,
			m_attachments[WORLDNORM_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			subresourceRange
		);

		VulkanImageBufferObject::TransitionImageLayout_(
			commandbuffer,
			m_attachments[ALBEDO_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			subresourceRange
		);
	}

	void MrtFBO::ClearColor(const VkClearColorValue & worldpos_clearvalue, const VkClearColorValue & worldnorm_clearvalue, const VkClearColorValue & albedo_clearvalue)
	{
		m_clearvalues[WORLDPOS_ATTACHMENT].color = worldpos_clearvalue;
		m_clearvalues[WORLDNORM_ATTACHMENT].color = worldnorm_clearvalue;
		m_clearvalues[ALBEDO_ATTACHMENT].color = albedo_clearvalue;
	}

	void MrtFBO::ClearDepthStencil(VkClearDepthStencilValue & cleardepthstencil)
	{
		m_clearvalues[DEPTH_ATTACHMENT].depthStencil = cleardepthstencil;
	}

	void MrtFBO::CreateRenderPass_()
	{
		// only called once
		auto lambda = [&]() {

			std::array<VkAttachmentDescription, 4> attachmentDescs = {};

			// Init attachment properties
			for(uint32_t i = 0; i < ALL_ATTACHMENTS; ++i)
			{
				attachmentDescs[i].samples			= VK_SAMPLE_COUNT_1_BIT;
				attachmentDescs[i].loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDescs[i].storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDescs[i].stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDescs[i].stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;

				if (i == DEPTH_ATTACHMENT)
				{
					attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // images as a depth
				}
				else
				{
					attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
					attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // images to be used later for other passes
				}

				// formats
				attachmentDescs[i].format = m_attachments[i].format;
			}

			VkSubpassDescription subPass{};
			std::vector<VkAttachmentReference> colorReferences;
			VkAttachmentReference depthAttachmentRef{};

			{
				colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
				colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
				colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

				depthAttachmentRef.attachment	= 3; // index ref depthAttachment (above) 
				depthAttachmentRef.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // images used as depth attachment

				subPass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
				subPass.colorAttachmentCount	= static_cast<uint32_t>(colorReferences.size());
				subPass.pColorAttachments		= colorReferences.data();
				subPass.pDepthStencilAttachment	= &depthAttachmentRef;
			}

			// Use subpass dependencies for attachment layout transitions
			VkSubpassDependency dependency;

			{
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0; // index 0 refer to our subPass .. which is the first and only one
				dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // wait until the end of shader
				dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
				dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT; // reading in the last pipeline stage
				dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				dependency.dependencyFlags	= VK_DEPENDENCY_BY_REGION_BIT;
			}

			VkRenderPassCreateInfo renderpass_create_info{};
			renderpass_create_info.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderpass_create_info.attachmentCount	= static_cast<uint32_t>(attachmentDescs.size());
			renderpass_create_info.pAttachments		= attachmentDescs.data();
			renderpass_create_info.subpassCount		= 1;
			renderpass_create_info.pSubpasses		= &subPass;
			renderpass_create_info.dependencyCount	= 1;
			renderpass_create_info.pDependencies	= &dependency;

			DebugLog::EC(vkCreateRenderPass(m_logicaldevice, &renderpass_create_info, nullptr, &m_renderpass));
		};

		std::call_once(m_sflag, lambda);

		if (m_renderpass == VK_NULL_HANDLE)
		{
			DebugLog::throwEx("render pass is not available");
			return;
		}
	}

	void MrtFBO::SetAttachment_(const VulkanImageBufferObject * image, E_ATTACHMENTS att)
	{
		FramebufferAttachment attach{};
		attach.image = image->getImage();
		attach.format = image->getFormat();
		attach.view = image->getImageView();
		m_attachments[att] = attach;
	}

	void MrtFBO::Destroy()
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
