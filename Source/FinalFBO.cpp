#include "FinalFBO.h"
#include "DebugLog.h"
#include <array>

namespace luna
{
	std::once_flag FinalFBO::m_sflag{};
	VkRenderPass FinalFBO::m_renderpass = VK_NULL_HANDLE;

	FinalFBO::FinalFBO()
	{
		m_attachments.resize(ALL_ATTACHMENTS);
		m_clearvalues.resize(ALL_ATTACHMENTS);
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

		VkImageView imageviews[] = {m_attachments[COLOR_ATTACHMENT].view};

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

	void FinalFBO::Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent)
	{
		// transition the image layout for the swapchain image
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

	void FinalFBO::TransitionAttachmentImagesLayout_(const VkCommandBuffer & commandbuffer)
	{
		// make sure it is optimal for the swap chain images
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer	= 0;
		barrier.subresourceRange.layerCount = 1;
		// transition the image layout for the swapchain image 
		barrier.image = m_attachments[COLOR_ATTACHMENT].image;

		vkCmdPipelineBarrier(
			commandbuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
	}

	void FinalFBO::SetColorAttachment(const VkImage& image, const VkImageView & view, const VkFormat & format)
	{
		FramebufferAttachment att{};
		att.image = image;
		att.format = format;
		att.view = view;

		m_attachments[COLOR_ATTACHMENT] = att;
	}

	void FinalFBO::ClearColor(const VkClearColorValue & clearvalue)
	{
		m_clearvalues[COLOR_ATTACHMENT].color = clearvalue;
	}

	void FinalFBO::CreateRenderPass_()
	{
		auto lambda = [&]() {
			VkAttachmentDescription attachmentDesc{};
			{
				attachmentDesc.format = m_attachments[COLOR_ATTACHMENT].format;
				attachmentDesc.samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDesc.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				attachmentDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentDesc.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // images to be presented in the swap chain
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
