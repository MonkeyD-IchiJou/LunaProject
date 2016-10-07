#include "LightPassFBO.h"
#include "TextureResources.h"
#include "VulkanTexture2D.h"
#include "DebugLog.h"

namespace luna
{
	std::once_flag LightPassFBO::m_sflag{};
	VkRenderPass LightPassFBO::m_renderpass = VK_NULL_HANDLE;

	LightPassFBO::LightPassFBO()
	{
		m_attachments.resize(LIGHTPASS_FBOATTs::ALL_ATTACHMENTS);
		m_clearvalues.resize(LIGHTPASS_FBOATTs::ALL_ATTACHMENTS);
	}

	LightPassFBO::~LightPassFBO()
	{
	}

	void LightPassFBO::Init(const VkExtent2D & extent)
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

		// create the attachment ourself
		imageattachment = &texrsc->Textures[eTEXTURES::LIGHTPROCESS_2D_RGBA8UNORM];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height, 
			VK_FORMAT_R8G8B8A8_UNORM, 
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment(*imageattachment, LIGHTPASS_FBOATTs::COLOR_ATTACHMENT);

		VkImageView imageviews[] = { 
			m_attachments[LIGHTPASS_FBOATTs::COLOR_ATTACHMENT].view
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

	void LightPassFBO::Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent)
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

	void LightPassFBO::TransitionAttachmentImagesLayout_(const VkCommandBuffer & commandbuffer)
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
			m_attachments[LIGHTPASS_FBOATTs::COLOR_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			subresourceRange
		);
	}

	void LightPassFBO::CreateRenderPass_()
	{
		auto lambda = [&]() {
			VkAttachmentDescription attachmentDesc{};
			{
				attachmentDesc.format = m_attachments[LIGHTPASS_FBOATTs::COLOR_ATTACHMENT].format;
				attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // images to be used for other shaders
			}

			VkSubpassDescription subPass{};
			VkAttachmentReference colorAttachmentRef{};
			{
				colorAttachmentRef.attachment = 0; // index ref colorAttachment (above) 
				colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // images used as color attachment

				subPass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subPass.colorAttachmentCount = 1;
				subPass.pColorAttachments = &colorAttachmentRef;
			}

			VkSubpassDependency dependency{};
			{
				dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
				dependency.dstSubpass = 0; // index 0 refer to our subPass .. which is the first and only one
				dependency.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // need to wait for the swap chain to finsh reading the image
				dependency.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT; // reading in the last pipeline stage
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

	void LightPassFBO::Destroy()
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
