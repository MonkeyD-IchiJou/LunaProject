#include "Renderer.h"
#include "DebugLog.h"
#include <array>

#include "VulkanSwapchain.h"
#include "BaseFBO.h"
#include "SimpleShader.h"
#include "WinNative.h"
#include "Model.h"
#include "SSBO.h"
#include "UBO.h"

#include "ModelResources.h"
#include "TextureResources.h"
#include "VulkanImageBufferObject.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm\glm.hpp>

#define INSTANCE_COUNT 2

namespace luna
{
	std::once_flag Renderer::m_sflag{};
	Renderer* Renderer::m_instance = VK_NULL_HANDLE;

	Renderer::Renderer()
	{
		// vulkan instance and logical device will be created in the parents constructor
	}

	void Renderer::CreateResources()
	{
		if (m_swapchain == nullptr)
		{
			// all the models resources will be init 
			ModelResources* models = ModelResources::getInstance();
			m_model = models->Models[eMODELS::CUBE_MODEL];

			// all the textures resources will be init
			TextureResources* textures = TextureResources::getInstance();

			// ubo and ssbo that are unique to this renderer
			m_ubo = new UBO();
			m_instance_ssbo = new SSBO();

			/* initial update for ssbo instancedata */
			std::vector<InstanceData> instancedata(INSTANCE_COUNT);
			instancedata[0].model = glm::mat4();
			instancedata[1].model = glm::translate(glm::mat4(), glm::vec3(0, 2.f, 0));
			m_instance_ssbo->Update(instancedata);

			uint32_t width = 0, height = 0;

			// swap chain creation
			{
				width = WinNative::getInstance()->getWinSizeX();
				height = WinNative::getInstance()->getWinSizeY();

				// create the swap chain for presenting images
				m_swapchain = new VulkanSwapchain();
				m_swapchain->CreateResources(width, height);
			}

			// create the render pass with the info from swap chain images
			InitFinalRenderPass_();

			// create framebuffer for each swapchain images
			m_fbos.resize(m_swapchain->getImageCount());
			
			for (int i = 0; i < m_fbos.size(); i++)
			{
				m_fbos[i] = new BaseFBO();
				m_fbos[i]->SetColorAttachment(m_swapchain->m_buffers[i].imageview, m_swapchain->getColorFormat());
				m_fbos[i]->SetDepthAttachment(textures->Textures[DEPTH_2D_ATTACHMENT]->getImageView(), textures->Textures[DEPTH_2D_ATTACHMENT]->getFormat());
				m_fbos[i]->SetRenderPass(m_renderpass);
				m_fbos[i]->Init({width, height});
			}

			// create the command pool first
			{
				VkCommandPoolCreateInfo commandPool_createinfo{};
				commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				commandPool_createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
				commandPool_createinfo.queueFamilyIndex = m_queuefamily_index.graphicsFamily;

				DebugLog::EC(vkCreateCommandPool(m_logicaldevice, &commandPool_createinfo, nullptr, &m_commandPool));
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

			// shader creation
			{
				m_shader = new SimpleShader();
				m_shader->setUBO(m_ubo);
				m_shader->setSSBO(m_instance_ssbo);
				m_shader->Init(m_renderpass);
			}

			// create semaphores for presetation and rendering synchronisation
			{
				VkSemaphoreCreateInfo semaphore_createInfo{};
				semaphore_createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_imageAvailableSemaphore));
				DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_renderFinishSemaphore));
			}
		}
	}

	void Renderer::InitFinalRenderPass_()
	{
		std::array<VkAttachmentDescription, 2> attachments;
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

			VkAttachmentDescription depthAttachment{};
			depthAttachment.format								= TextureResources::getInstance()->Textures[DEPTH_2D_ATTACHMENT]->getFormat();
			depthAttachment.samples								= VK_SAMPLE_COUNT_1_BIT;
			depthAttachment.loadOp								= VK_ATTACHMENT_LOAD_OP_CLEAR;
			depthAttachment.storeOp								= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.stencilLoadOp						= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			depthAttachment.stencilStoreOp						= VK_ATTACHMENT_STORE_OP_DONT_CARE;
			depthAttachment.initialLayout						= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthAttachment.finalLayout							= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // images as a depth

			attachments = { colorAttachment, depthAttachment };
		}

		VkSubpassDescription subPass{};
		VkAttachmentReference colorAttachmentRef{};
		VkAttachmentReference depthAttachmentRef{};
		{
			colorAttachmentRef.attachment						= 0; // index ref colorAttachment (above) 
			colorAttachmentRef.layout							= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // images used as color attachment

			depthAttachmentRef.attachment						= 1; // index ref depthAttachment (above) 
			depthAttachmentRef.layout							= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // images used as depth attachment

			subPass.pipelineBindPoint							= VK_PIPELINE_BIND_POINT_GRAPHICS;
			subPass.colorAttachmentCount						= 1;
			subPass.pColorAttachments							= &colorAttachmentRef;
			subPass.pDepthStencilAttachment						= &depthAttachmentRef;
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

	void Renderer::Record()
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

		// can record the command buffer liao
		// these command buffer is for the final rendering and final presentation on the screen 
		for (size_t i = 0; i < m_commandbuffers.size(); ++i)
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;

			vkBeginCommandBuffer(m_commandbuffers[i], &beginInfo);

			// transition the image layout for the swapchain image 
			barrier.image = m_swapchain->m_buffers[i].image;
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

			// push the constant
			m_shader->LoadOffset(m_commandbuffers[i], 0);

			// Drawing start
			m_model->DrawInstanced(m_commandbuffers[i], INSTANCE_COUNT);

			// unbind the fbo
			m_fbos[i]->UnBind(m_commandbuffers[i]);

			// finish recording
			DebugLog::EC(vkEndCommandBuffer(m_commandbuffers[i]));
		}
	}

	void Renderer::Render()
	{
		{
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.f;

			/*{
				std::vector<InstanceData> instancedata(INSTANCE_COUNT);
				instancedata[0].model = glm::rotate(glm::mat4(), time * glm::radians(40.f), glm::vec3(0.f, 1.f, 0.f));;
				instancedata[1].model = glm::translate(glm::mat4(), glm::vec3(1.f, 2.f, 0.f));
				m_instance_ssbo->Update(instancedata);
			}*/

			UBOData data{};
			data.view = glm::lookAt(glm::vec3(0.f, 0.f, 9.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
			data.proj = glm::perspective(glm::radians(45.f), 1080.f / 720.f, 0.1f, 10.0f); // take note .. hardcoded aspects
			
			// Vulkan clip space has inverted Y and half Z.
			glm::mat4 Clip = glm::mat4(
				1.0f,  0.0f, 0.0f, 0.0f,
				0.0f, -1.0f, 0.0f, 0.0f,
				0.0f,  0.0f, 0.5f, 0.0f,
				0.0f,  0.0f, 0.5f, 1.0f
			);

			data.proj = Clip * data.proj;

			m_ubo->Update(data);
		}

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
		VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitDstStageMask = waitStages; // wait until draw finish
		vkQueueSubmit(m_graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);

		// present it on the screen pls
		m_swapchain->QueuePresent(m_graphic_queue, imageindex, m_renderFinishSemaphore);
	}

	void Renderer::CleanUpResources()
	{
		// wait until gpu finish pls
		if (m_logicaldevice != VK_NULL_HANDLE)
		{
			vkDeviceWaitIdle(m_logicaldevice);
		}

		// destroy all the resources
		ModelResources::getInstance()->Destroy();
		TextureResources::getInstance()->Destroy();

		if (m_ubo != nullptr)
		{
			delete m_ubo;
			m_ubo = nullptr;
		}

		if (m_instance_ssbo != nullptr)
		{
			delete m_instance_ssbo;
			m_instance_ssbo = nullptr;
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
	}
}
