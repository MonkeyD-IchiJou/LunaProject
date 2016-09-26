#include "Renderer.h"
#include "DebugLog.h"
#include <array>

#include "VulkanSwapchain.h"
#include "BaseFBO.h"
#include "BasicShader.h"
#include "WinNative.h"
#include "ResourceManager.h"
#include "Model.h"

#include "BasicUBO.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

namespace luna
{
	std::once_flag Renderer::m_sflag{};
	Renderer* Renderer::m_instance = VK_NULL_HANDLE;

	Renderer::Renderer()
	{
		// vulkan instance and logical device will be created in the parents constructor
	}

	Renderer::~Renderer()
	{
	}

	void Renderer::CreateResources()
	{
		// all the resources will be init 
		ResourceManager* resource = ResourceManager::getInstance();
		m_quad = resource->Models[eMODELS::QUAD_MODEL];
		m_ubo = resource->UBO;

		if (m_swapchain == nullptr)
		{
			uint32_t width = WinNative::getInstance()->getWinSizeX();
			uint32_t height = WinNative::getInstance()->getWinSizeY();

			// create the swap chain for presenting images
			m_swapchain = new VulkanSwapchain();
			m_swapchain->CreateResources(width, height);

			// create the render pass with the info from swap chain images
			InitFinalRenderPass_();

			// create framebuffer for each swapchain images
			m_fbos.resize(m_swapchain->getImageCount());
			for (int i = 0; i < m_fbos.size(); i++)
			{
				m_fbos[i] = new BaseFBO();
				m_fbos[i]->AddColorAttachment(m_swapchain->m_buffers[i].imageview, m_swapchain->getColorFormat());
				m_fbos[i]->AddRenderPass(m_renderpass);
				m_fbos[i]->Init({width, height});
			}

			// shader 
			m_shader = new BasicShader();
			m_shader->Init(m_renderpass);
		}
	}

	void Renderer::InitFinalRenderPass_()
	{
		std::array<VkAttachmentDescription, 1> attachments;
		{
			VkAttachmentDescription colorAttachment{};
			colorAttachment.format								= m_swapchain->getColorFormat();
			colorAttachment.samples								= VK_SAMPLE_COUNT_1_BIT;
			colorAttachment.loadOp								= VK_ATTACHMENT_LOAD_OP_CLEAR;
			colorAttachment.storeOp								= VK_ATTACHMENT_STORE_OP_STORE;
			colorAttachment.stencilLoadOp						= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			colorAttachment.stencilStoreOp						= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			colorAttachment.initialLayout						= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			colorAttachment.finalLayout							= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // images to be presented in the swap chain

			attachments = { colorAttachment };
		}

		VkSubpassDescription subPass{};
		VkAttachmentReference colorAttachmentRef{};
		{
			colorAttachmentRef.attachment						= 0; // index ref colorAttachment (above) 
			colorAttachmentRef.layout							= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // images used as color attachment

			subPass.pipelineBindPoint							= VK_PIPELINE_BIND_POINT_GRAPHICS;
			subPass.colorAttachmentCount						= 1;
			subPass.pColorAttachments							= &colorAttachmentRef;
		}

		VkSubpassDependency dependency{};
		{
			dependency.srcSubpass								= VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass								= 0; // index 0 refer to our subPass .. which is the first and only one
			dependency.srcStageMask								= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT; // need to wait for the swap chain to finsh reading the image
			dependency.srcAccessMask							= VK_ACCESS_MEMORY_READ_BIT; // reading in the last pipeline stage
			dependency.dstStageMask								= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstAccessMask							= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		}

		VkRenderPassCreateInfo renderpass_create_info{};
		renderpass_create_info.sType						= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderpass_create_info.attachmentCount				= (uint32_t) attachments.size();
		renderpass_create_info.pAttachments					= attachments.data();
		renderpass_create_info.subpassCount					= 1;
		renderpass_create_info.pSubpasses					= &subPass;
		renderpass_create_info.dependencyCount				= 1;
		renderpass_create_info.pDependencies				= &dependency;

		DebugLog::EC(vkCreateRenderPass(m_logicaldevice, &renderpass_create_info, nullptr, &m_renderpass));
	}

	void Renderer::CleanUpResources()
	{
		// wait until gpu finish pls
		if (m_logicaldevice != VK_NULL_HANDLE)
		{
			vkDeviceWaitIdle(m_logicaldevice);
		}

		ResourceManager::getInstance()->Destroy();
		
		if (m_imageAvailableSemaphore != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_imageAvailableSemaphore, nullptr);
			m_imageAvailableSemaphore = VK_NULL_HANDLE;
		}
		if (m_renderFinishSemaphore != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_renderFinishSemaphore, nullptr);
			m_renderFinishSemaphore = VK_NULL_HANDLE;
		}

		if (m_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_logicaldevice, m_commandPool, nullptr);
			m_commandPool = VK_NULL_HANDLE;
		}

		if (m_shader != nullptr)
		{
			m_shader->Destroy();
			delete m_shader;
			m_shader = nullptr;
		}

		for (auto &fbo : m_fbos)
		{
			if (fbo != nullptr)
			{
				fbo->Destroy();
				delete fbo;
				fbo = nullptr;
			}
		}

		m_fbos.clear();

		if (m_renderpass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(m_logicaldevice, m_renderpass, nullptr);
			m_renderpass = VK_NULL_HANDLE;
		}
		
		if (m_swapchain != nullptr)
		{
			m_swapchain->Destroy();
			delete m_swapchain;
			m_swapchain = nullptr;
		}
	}

	void Renderer::RenderSetup()
	{
		/* create the command pool first */
		VkCommandPoolCreateInfo commandPool_createinfo{};
		commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPool_createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPool_createinfo.queueFamilyIndex = m_queuefamily_index.graphicsFamily;

		DebugLog::EC(vkCreateCommandPool(m_logicaldevice, &commandPool_createinfo, nullptr, &m_commandPool));

		// if the commandbuffers is not empty, free it
		if (m_commandbuffers.size() > 0)
		{
			vkFreeCommandBuffers(m_logicaldevice, m_commandPool,(uint32_t) m_commandbuffers.size(), m_commandbuffers.data());
		}

		// command buffer creation
		{
			m_commandbuffers.resize(m_fbos.size());

			VkCommandBufferAllocateInfo buffer_allocateInfo{};
			buffer_allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			buffer_allocateInfo.commandPool = m_commandPool;
			buffer_allocateInfo.commandBufferCount = (uint32_t)m_commandbuffers.size();
			buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, m_commandbuffers.data()));
		}

		// can record the command buffer liao
		// these command buffer is for the final rendering and final presentation on the screen 
		for (size_t i = 0; i < m_commandbuffers.size(); ++i)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;

			vkBeginCommandBuffer(m_commandbuffers[i], &beginInfo);

			// make sure it is optimal for the swap chain images
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
			barrier.image = m_swapchain->m_buffers[i].image;
			barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			barrier.subresourceRange.baseMipLevel = 0;
			barrier.subresourceRange.levelCount = 1;
			barrier.subresourceRange.baseArrayLayer	= 0;
			barrier.subresourceRange.layerCount = 1;

			vkCmdPipelineBarrier(
				m_commandbuffers[i],
				VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
				0,
				0, nullptr,
				0, nullptr,
				1, &barrier
			);

			// starting a render pass 
			// bind the fbo
			m_fbos[i]->Bind(m_commandbuffers[i]);

			// bind the shader pipeline stuff
			m_shader->Bind(m_commandbuffers[i]);

			// Drawing start
			m_quad->Draw(m_commandbuffers[i]);
			ResourceManager::getInstance()->Models[eMODELS::BUNNY_MODEL]->Draw(m_commandbuffers[i]);
			//ResourceManager::getInstance()->Models[eMODELS::TYRA_MODEL]->Draw(m_commandbuffers[i]);

			// unbind the fbo
			m_fbos[i]->UnBind(m_commandbuffers[i]);

			// finish recording
			DebugLog::EC(vkEndCommandBuffer(m_commandbuffers[i]));
		}

		VkSemaphoreCreateInfo semaphore_createInfo{};
		semaphore_createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_imageAvailableSemaphore));
		DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_renderFinishSemaphore));
	}

	void Renderer::Render()
	{
		static auto startTime = std::chrono::high_resolution_clock::now();
		auto currentTime = std::chrono::high_resolution_clock::now();
		float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.f;

		UBOData data{};
		data.model = glm::rotate(glm::mat4(), time * glm::radians(10.f), glm::vec3(0.f, 1.f, 0.f));
		data.view = glm::lookAt(glm::vec3(0.f, 1.f, 8.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		data.proj = glm::perspective(glm::radians(45.f), 1080.f / 720.f, 0.1f, 10.0f); // take note .. hardcoded aspects
		m_ubo->Update(data);


		VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		uint32_t imageindex = 0;
		m_swapchain->AcquireNextImage(m_imageAvailableSemaphore, &imageindex);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_commandbuffers[imageindex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &m_renderFinishSemaphore;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &m_imageAvailableSemaphore;
		submitInfo.pWaitDstStageMask = waitStages; // wait until draw finish
		vkQueueSubmit(m_graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);
		
		// present it on the screen pls
		m_swapchain->QueuePresent(m_graphic_queue, imageindex, m_renderFinishSemaphore);
	}
}
