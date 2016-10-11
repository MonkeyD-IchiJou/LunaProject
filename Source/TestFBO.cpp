#include "TestFBO.h"
#include "DebugLog.h"
#include <array>
#include "TextureResources.h"
#include "VulkanTexture2D.h"

namespace luna
{
	std::once_flag TestFBO::m_sflag{};
	VkRenderPass TestFBO::m_renderpass = VK_NULL_HANDLE;

	TestFBO::TestFBO()
	{
		m_attachments.resize(TEST_FBOATTs::ALL_ATTACHMENTS);
		m_clearvalues.resize(TEST_FBOATTs::ALL_ATTACHMENTS);
	}

	TestFBO::~TestFBO()
	{
	}

	void TestFBO::Init(const VkExtent2D & extent)
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
		
		// first attachment, used for first subpass to write on it, then second subpass to read on it
		imageattachment = &texrsc->Textures[eTEXTURES::SUBPASSFIRST_2D_RGBA8UNORM];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment(*imageattachment, TEST_FBOATTs::COLOR_ATTACHMENT); // first attachment for first subpass output color, and input for second subpass
		
		// second attachment, used to store the final results as a textures
		imageattachment = &texrsc->Textures[eTEXTURES::SUBPASSSECOND_2D_RGBA8UNORM];
		*imageattachment = new VulkanTexture2D(
			m_resolution.width, m_resolution.height,
			VK_FORMAT_R8G8B8A8_UNORM,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		);
		SetAttachment(*imageattachment, TEST_FBOATTs::PRESENT_ATTACHMENT); // second attachment for second subpass output color

		VkImageView imageviews[] = {
			m_attachments[TEST_FBOATTs::COLOR_ATTACHMENT].view, 
			m_attachments[TEST_FBOATTs::PRESENT_ATTACHMENT].view
		};

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

	void TestFBO::Bind(const VkCommandBuffer & commandbuffer, VkSubpassContents subpasscontent)
	{
		VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
			commandbuffer, m_attachments[TEST_FBOATTs::COLOR_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
			VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
		);

		VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
			commandbuffer, m_attachments[TEST_FBOATTs::PRESENT_ATTACHMENT].image,
			VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, 
			VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
		);

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

	void TestFBO::CreateRenderPass_()
	{
		auto lambda = [&]() {

			std::array<VkAttachmentDescription, 2> attachmentDesc{};

			{
				attachmentDesc[0].format = m_attachments[TEST_FBOATTs::COLOR_ATTACHMENT].format;
				attachmentDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // dun care about storing this attachment
				attachmentDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentDesc[0].finalLayout = VK_IMAGE_LAYOUT_GENERAL; // for second subpass to input it

				// final color to present
				attachmentDesc[1].format = m_attachments[TEST_FBOATTs::PRESENT_ATTACHMENT].format;
				attachmentDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
				attachmentDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
				attachmentDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE; // store this attachment for the next fbo to render it
				attachmentDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
				attachmentDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
				attachmentDesc[1].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				attachmentDesc[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			}

			// i have 2 subpass
			std::array<VkSubpassDescription, 2> subPass{};
			VkAttachmentReference outputAttachmentRef{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};
			VkAttachmentReference inputAttachmentRef{0, VK_IMAGE_LAYOUT_GENERAL};
			VkAttachmentReference presentColorAttachmentRef{1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

			{
				// first subpass
				// this subpass only output 1 image 
				subPass[0].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subPass[0].colorAttachmentCount = 1;
				subPass[0].pColorAttachments = &outputAttachmentRef;
				subPass[0].inputAttachmentCount = 0;
				subPass[0].pInputAttachments = nullptr;
				subPass[0].preserveAttachmentCount = 0;
				subPass[0].pPreserveAttachments = nullptr;
				subPass[0].pDepthStencilAttachment = nullptr;
				subPass[0].pResolveAttachments = nullptr;

				// second subpass
				// this subpass only input 1 attachment from prev subpass, then output to the presentattachment image 
				subPass[1].pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
				subPass[1].colorAttachmentCount = 1;
				subPass[1].pColorAttachments = &presentColorAttachmentRef; // only got 1 color attachment, layout = 0 in shaders
				subPass[1].inputAttachmentCount = 1;
				subPass[1].pInputAttachments = &inputAttachmentRef;
				subPass[1].preserveAttachmentCount = 0;
				subPass[1].pPreserveAttachments = nullptr;
				subPass[1].pDepthStencilAttachment = nullptr;
				subPass[1].pResolveAttachments = nullptr;
			}

			VkSubpassDependency dependency{};
			dependency.dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
			dependency.srcSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependency.dstSubpass = 1;
			dependency.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			dependency.dstAccessMask = VK_ACCESS_INPUT_ATTACHMENT_READ_BIT;

			// Render pass create
			VkRenderPassCreateInfo renderpass_create_info{};
			renderpass_create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderpass_create_info.attachmentCount = static_cast<uint32_t>(attachmentDesc.size());
			renderpass_create_info.pAttachments = attachmentDesc.data();
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

	void TestFBO::Destroy()
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
