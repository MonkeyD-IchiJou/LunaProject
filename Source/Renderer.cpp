#include "Renderer.h"
#include "DebugLog.h"
#include <array>

#include "VulkanSwapchain.h"
#include "WinNative.h"
#include "Model.h"
#include "SSBO.h"
#include "UBO.h"

#include "MrtFBO.h"
#include "FinalFBO.h"
#include "MRTShader.h"
#include "FinalPassShader.h"

#include "ModelResources.h"
#include "TextureResources.h"
#include "VulkanImageBufferObject.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm\glm.hpp>

#define INSTANCE_COUNT 3

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
			instancedata[0].model = glm::rotate(glm::mat4(), 45.f, glm::vec3(0, 1.f, 0)); instancedata[0].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[0].model));
			instancedata[1].model = glm::translate(glm::mat4(), glm::vec3(0, 2.f, 0)); instancedata[1].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[1].model));
			instancedata[2].model = glm::translate(glm::mat4(), glm::vec3(0, 0.f, 2.f)); instancedata[2].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[2].model));
			m_instance_ssbo->Update(instancedata);

			/* create all the framebuffers, swapchain and shaders */
			FramebuffersCreation_();

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
			
				// offscreen command buffer
				buffer_allocateInfo.commandBufferCount = 1;
				DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_offscreen_cmdbuffer));
			}

			// create semaphores for presetation and rendering synchronisation
			{
				VkSemaphoreCreateInfo semaphore_createInfo{};
				semaphore_createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
				DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_presentComplete));
				DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_renderComplete));
				DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_offscreenComplete));
			}
		}
	}

	void luna::Renderer::FramebuffersCreation_()
	{
		// all the textures resources will be init
		TextureResources* texrsc = TextureResources::getInstance();

		// create the framebuffer for mrt pass
		m_mrtfbo = new MrtFBO();
		VkClearDepthStencilValue depthStencil = { 1.0f, 0 };
		m_mrtfbo->ClearDepthStencil(depthStencil);
		m_mrtfbo->ClearColor({0.f, 0.f, 1.f, 1.f}, {0.f, 0.f, 0.f, 1.f}, {0.f, 0.f, 0.f, 1.f});
		m_mrtfbo->Init({1920, 1080}); // make sure the attachments are the same size too

		// swap chain and final fbo creation
		{
			// create the swap chain for presenting images
			m_swapchain = new VulkanSwapchain();
			m_swapchain->Init();

			// create framebuffer for each swapchain images
			m_fbos.resize(m_swapchain->getImageCount());
			for (int i = 0; i < m_fbos.size(); i++)
			{
				m_fbos[i] = new FinalFBO();
				m_fbos[i]->SetColorAttachment(m_swapchain->m_buffers[i].image, m_swapchain->m_buffers[i].imageview, m_swapchain->getColorFormat());
				m_fbos[i]->ClearColor({ 1.f, 1.f, 1.f, 1.f });
				m_fbos[i]->Init(m_swapchain->getExtent()); // must be the same as swap chain extent
			}
		}

		// shader creation
		{
			m_mrtshader = new MRTShader();
			m_mrtshader->Init(MrtFBO::getRenderPass());
			m_mrtshader->UpdateDescriptorSets(m_ubo, m_instance_ssbo, texrsc->Textures[eTEXTURES::BASIC_2D_BC2]);

			m_finalpassshader = new FinalPassShader();
			m_finalpassshader->Init(FinalFBO::getRenderPass());
			m_finalpassshader->UpdateDescriptorSets(texrsc->Textures[eTEXTURES::ALBEDO_2D_RGBA8UNORM]);
		}
	}

	void luna::Renderer::RecordMRTOffscreen_()
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		// start recording the offscreen command buffers
		vkBeginCommandBuffer(m_offscreen_cmdbuffer, &beginInfo);

		// bind the fbo
		m_mrtfbo->Bind(m_offscreen_cmdbuffer);

		m_mrtshader->Bind(m_offscreen_cmdbuffer);
		m_mrtshader->SetViewPort(m_offscreen_cmdbuffer, m_mrtfbo->getResolution().width, m_mrtfbo->getResolution().height);
		m_mrtshader->LoadObjectOffset(m_offscreen_cmdbuffer, 0);
		
		// draw objects
		m_model->DrawInstanced(m_offscreen_cmdbuffer, INSTANCE_COUNT);
		
		m_mrtfbo->UnBind(m_offscreen_cmdbuffer);

		// finish recording
		DebugLog::EC(vkEndCommandBuffer(m_offscreen_cmdbuffer));
	}

	void luna::Renderer::RecordFinalFrame_()
	{	
		// can record the command buffer liao
		// these command buffer is for the final rendering and final presentation on the screen 
		for (size_t i = 0; i < m_commandbuffers.size(); ++i)
		{
			const VkCommandBuffer& commandbuffer = m_commandbuffers[i];
			FinalFBO* finalfbo = m_fbos[i];

			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
			beginInfo.pInheritanceInfo = nullptr;

			vkBeginCommandBuffer(commandbuffer, &beginInfo);

			// starting a render pass 
			// bind the fbo
			finalfbo->Bind(commandbuffer);

			// bind the shader pipeline stuff
			m_finalpassshader->Bind(commandbuffer);
			m_finalpassshader->SetViewPort(commandbuffer, finalfbo->getResolution().width, finalfbo->getResolution().height);

			// Draw the quad
			ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(commandbuffer);

			// unbind the fbo
			finalfbo->UnBind(commandbuffer);

			// finish recording
			DebugLog::EC(vkEndCommandBuffer(commandbuffer));
		}
	}

	void Renderer::Record()
	{
		RecordMRTOffscreen_();
		RecordFinalFrame_();
	}

	void Renderer::Render()
	{
		{
			static auto startTime = std::chrono::high_resolution_clock::now();
			auto currentTime = std::chrono::high_resolution_clock::now();
			float time = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - startTime).count() / 1000.f;

			{
				std::vector<InstanceData> instancedata(INSTANCE_COUNT);
				instancedata[0].model = glm::rotate(glm::mat4(), time * glm::radians(40.f), glm::vec3(0, 1.f, 0)); instancedata[0].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[0].model));
				instancedata[1].model = glm::translate(glm::mat4(), glm::vec3(0, 2.f, 0)); instancedata[1].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[1].model));
				instancedata[2].model = glm::translate(glm::mat4(), glm::vec3(0, 0.f, 2.f)); instancedata[2].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[2].model));
				m_instance_ssbo->Update(instancedata);
			}

			UBOData data{};
			data.view = glm::lookAt(glm::vec3(-2.f, 2.f, -5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
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

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.waitSemaphoreCount = 1;
		VkPipelineStageFlags waitStages[1] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitDstStageMask = waitStages; // wait until draw finish

		// get the available image to render with
		uint32_t imageindex = 0;
		m_swapchain->AcquireNextImage(m_presentComplete, &imageindex);

		// MRT rendering job submit
		submitInfo.pWaitSemaphores = &m_presentComplete; // wait for swap chain present finish
		submitInfo.pSignalSemaphores = &m_offscreenComplete; // will tell the final pass to render, when i render finish
		submitInfo.pCommandBuffers = &m_offscreen_cmdbuffer;
		vkQueueSubmit(m_graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);
		
		// final image submiting after MRT pass
		submitInfo.pWaitSemaphores = &m_offscreenComplete; // wait for MRT render finish
		submitInfo.pSignalSemaphores = &m_renderComplete; // will tell the swap chain to present, when i render finish
		submitInfo.pCommandBuffers = &m_commandbuffers[imageindex];
		vkQueueSubmit(m_graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);

		// present it on the screen pls
		m_swapchain->QueuePresent(m_graphic_queue, imageindex, m_renderComplete);
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
		
		if (m_mrtshader != nullptr)
		{
			m_mrtshader->Destroy();
			delete m_mrtshader;
			m_mrtshader = nullptr;
		}

		if (m_finalpassshader != nullptr)
		{
			m_finalpassshader->Destroy();
			delete m_finalpassshader;
			m_finalpassshader = nullptr;
		}

		if (m_mrtfbo != nullptr)
		{
			m_mrtfbo->Destroy();
			delete m_mrtfbo;
			m_mrtfbo = nullptr;
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

		if (m_swapchain != nullptr)
		{
			m_swapchain->Destroy();
			delete m_swapchain;
			m_swapchain = nullptr;
		}

		if (m_presentComplete != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_presentComplete, nullptr);
			m_presentComplete = VK_NULL_HANDLE;
		}
		if (m_renderComplete != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_renderComplete, nullptr);
			m_renderComplete = VK_NULL_HANDLE;
		}
		if (m_offscreenComplete != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_offscreenComplete, nullptr);
			m_offscreenComplete = VK_NULL_HANDLE;
		}

		if (m_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_logicaldevice, m_commandPool, nullptr);
			m_commandPool = VK_NULL_HANDLE;
		}
	}
}
