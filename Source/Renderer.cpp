#include "Renderer.h"
#include "DebugLog.h"
#include <array>

#include "VulkanSwapchain.h"
#include "SSBO.h"
#include "UBO.h"

#include "MrtFBO.h"
#include "LightPassFBO.h"
#include "FinalFBO.h"

#include "MRTShader.h"
#include "DirLightPassShader.h"
#include "FinalPassShader.h"
#include "TextShader.h"

#include "ModelResources.h"
#include "Model.h"
#include "TextureResources.h"
#include "VulkanImageBufferObject.h"
#include "Font.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm\glm.hpp>

#define INSTANCE_COUNT 3
static uint32_t totaltext = 0;

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
			TextureResources* texrsc = TextureResources::getInstance();

			// swap chain and final fbo creation
			// create the swap chain for presenting images
			m_swapchain = new VulkanSwapchain();
			m_swapchain->Init();

			// ubo and ssbo that are unique to this renderer
			m_ubo = new UBO();
			m_instance_ssbo = new SSBO();
			m_fontinstance_ssbo = new SSBO();

			/* initial update for ssbo instancedata */
			std::vector<InstanceData> instancedata(INSTANCE_COUNT);
			instancedata[0].model = glm::rotate(glm::mat4(), 45.f, glm::vec3(0, 1.f, 0)); 
			instancedata[0].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[0].model));
			instancedata[1].model = glm::translate(glm::mat4(), glm::vec3(0, 2.f, 0)); 
			instancedata[1].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[1].model));
			instancedata[2].model = glm::translate(glm::mat4(), glm::vec3(0, 0.f, 2.f)); 
			instancedata[2].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[2].model));
			m_instance_ssbo->Update(instancedata);

			Font* font = new Font();
			font->LoadFonts(getAssetPath() + "Fonts/eva.fnt", (float)texrsc->Textures[eTEXTURES::EVAFONT_2D_BC3]->getWidth(), (float)texrsc->Textures[eTEXTURES::EVAFONT_2D_BC3]->getHeight());

			{
				glm::mat4 proj = glm::ortho(0.f, 1080.f, 720.f, 0.f); // scale with screen size
				glm::mat4 parentT = glm::translate(glm::mat4(), glm::vec3(500.f, 720.f / 2.f, 0.f));
				glm::mat4 parentR = glm::rotate(glm::mat4(), 0.f, glm::vec3(0, 0, 1.f));
				glm::mat4 parentS = glm::scale(glm::mat4(), glm::vec3(400.f, 400.f, 1.f));

				std::string str = "!@#$%^&*()?><:'{}[];'./, ";
				glm::vec2 cursor = {0.f, 0.0f};

				totaltext = static_cast<uint32_t>(str.size());
				std::vector<FontInstanceData> fontinstancedata(totaltext);

				for(uint32_t i = 0; i < totaltext; ++i)
				{
					FontInstanceData& fid = fontinstancedata[i];
					const vulkanchar& vc = font->vulkanChars[str[i]];

					// calc the top left hand corner position
					glm::vec2 toplefthandcorner = glm::vec2(cursor.x + vc.xoffset, cursor.y + vc.yoffset);

					// then find the correct position relative with the left hand corner
					glm::vec2 position = { toplefthandcorner.x + vc.halfsize.x, toplefthandcorner.y - vc.halfsize.y };

					glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(position, 0.f));
					glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(vc.size, 1.f));
					
					fid.transformation = proj * parentT * parentR * parentS * t * s;
					fid.uv[0] = vc.uv[0];
					fid.uv[1] = vc.uv[1];
					fid.uv[2] = vc.uv[2];
					fid.uv[3] = vc.uv[3];

					// next cursor pointing at
					cursor.x += vc.xadvance;

					// materials set up 
					fid.fontMaterials[0] = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f); // color
					fid.fontMaterials[1] = glm::vec4(0.45f, 0.15f, 0.5f, 0.15f); // properties
					fid.fontMaterials[2] = glm::vec4(0.55f, 0.23f, 0.1f, 0.f); // outline color
					fid.fontMaterials[3] = glm::vec4(0.0015f, 0.000f, 0.f, 0.f); // border offset
				}

				m_fontinstance_ssbo->Update(fontinstancedata);
			}

			delete font;

			VkClearValue clearvalue{};
			clearvalue.color = {0.f, 0.f, 0.f, 1.f};
			
			// create the framebuffer for mrt pass
			m_mrt_fbo = new MrtFBO();
			m_mrt_fbo->Clear(clearvalue, MRT_FBOATTs::WORLDPOS_ATTACHMENT);
			m_mrt_fbo->Clear(clearvalue, MRT_FBOATTs::WORLDNORM_ATTACHMENT);
			m_mrt_fbo->Clear(clearvalue, MRT_FBOATTs::ALBEDO_ATTACHMENT);
			clearvalue.depthStencil = { 1.0f, 0 };
			m_mrt_fbo->Clear(clearvalue, MRT_FBOATTs::DEPTH_ATTACHMENT);
			m_mrt_fbo->Init({1920, 1080}); // make sure the attachments are the same size too
			
			// create the framebuffer for light pass
			m_lightpass_fbo = new LightPassFBO();
			clearvalue.color = {1.0f, 1.0f, 1.0f, 1.f};
			m_lightpass_fbo->Clear(clearvalue, LIGHTPASS_FBOATTs::COLOR_ATTACHMENT);
			m_lightpass_fbo->Init({ 1920, 1080 });

			// create framebuffer for each swapchain images
			m_finalpass_fbos.resize(m_swapchain->getImageCount());
			for (int i = 0; i < m_finalpass_fbos.size(); i++)
			{
				m_finalpass_fbos[i] = new FinalFBO();
				m_finalpass_fbos[i]->SetAttachment(m_swapchain->m_buffers[i].image, m_swapchain->m_buffers[i].imageview, m_swapchain->getColorFormat(), FINAL_FBOATTs::COLOR_ATTACHMENT);
				m_finalpass_fbos[i]->Clear(clearvalue, FINAL_FBOATTs::COLOR_ATTACHMENT);
				m_finalpass_fbos[i]->Init(m_swapchain->getExtent()); // must be the same as swap chain extent
			}

			// shaders creation
			m_mrt_shader = new MRTShader();
			m_mrt_shader->Init(MrtFBO::getRenderPass());
			m_mrt_shader->UpdateDescriptorSets(m_ubo, m_instance_ssbo, texrsc->Textures[eTEXTURES::BASIC_2D_BC2]);

			m_dirlightpass_shader = new DirLightPassShader();
			m_dirlightpass_shader->Init(LightPassFBO::getRenderPass());
			m_dirlightpass_shader->UpdateDescriptorSets(
				texrsc->Textures[eTEXTURES::WORLDPOS_2D_RGBA16FLOAT], 
				texrsc->Textures[eTEXTURES::WORLDNORMAL_2D_RGBA16FLOAT], 
				texrsc->Textures[eTEXTURES::ALBEDO_2D_RGBA8UNORM]);

			m_finalpass_shader = new FinalPassShader();
			m_finalpass_shader->Init(FinalFBO::getRenderPass());
			m_finalpass_shader->UpdateDescriptorSets(texrsc->Textures[eTEXTURES::LIGHTPROCESS_2D_RGBA8UNORM]);

			m_text_shader = new TextShader();
			m_text_shader->Init(FinalFBO::getRenderPass());
			m_text_shader->UpdateDescriptorSets(m_fontinstance_ssbo, texrsc->Textures[eTEXTURES::EVAFONT_2D_BC3]);

			// create the command pool first
			VkCommandPoolCreateInfo commandPool_createinfo{};
			commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPool_createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPool_createinfo.queueFamilyIndex = m_queuefamily_index.graphicsFamily;
			DebugLog::EC(vkCreateCommandPool(m_logicaldevice, &commandPool_createinfo, nullptr, &m_commandPool));

			// command buffer creation
			m_finalpass_cmdbuffers.resize(m_finalpass_fbos.size());
			VkCommandBufferAllocateInfo buffer_allocateInfo{};
			buffer_allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			buffer_allocateInfo.commandPool = m_commandPool;
			buffer_allocateInfo.commandBufferCount = (uint32_t)m_finalpass_cmdbuffers.size();
			buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, m_finalpass_cmdbuffers.data()));

			buffer_allocateInfo.commandBufferCount = 1;
			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_mrt_cmdbuffer));
			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_lightpass_cmdbuffer));

			// create semaphores for presetation and rendering synchronisation
			VkSemaphoreCreateInfo semaphore_createInfo{};
			semaphore_createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_presentComplete));
			DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_renderComplete));
			DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_mrtComplete));
			DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_lightpassComplete));
		}
	}

	void luna::Renderer::RecordMRTOffscreen_()
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		// start recording the offscreen command buffers
		vkBeginCommandBuffer(m_mrt_cmdbuffer, &beginInfo);

		// bind the fbo
		m_mrt_fbo->Bind(m_mrt_cmdbuffer);

		m_mrt_shader->Bind(m_mrt_cmdbuffer);
		m_mrt_shader->SetViewPort(m_mrt_cmdbuffer, m_mrt_fbo->getResolution());
		m_mrt_shader->LoadObjectOffset(m_mrt_cmdbuffer, 0);
		
		// draw objects
		m_model->DrawInstanced(m_mrt_cmdbuffer, INSTANCE_COUNT);
		
		m_mrt_fbo->UnBind(m_mrt_cmdbuffer);

		// finish recording
		DebugLog::EC(vkEndCommandBuffer(m_mrt_cmdbuffer));
	}

	void Renderer::RecordLightPassOffscreen_()
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		// start recording the offscreen command buffers
		vkBeginCommandBuffer(m_lightpass_cmdbuffer, &beginInfo);

		// bind the fbo
		m_lightpass_fbo->Bind(m_lightpass_cmdbuffer);

		// shader binding
		m_dirlightpass_shader->Bind(m_lightpass_cmdbuffer);
		m_dirlightpass_shader->SetViewPort(m_lightpass_cmdbuffer, m_lightpass_fbo->getResolution());
		
		// Draw the quad
		ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(m_lightpass_cmdbuffer);

		m_lightpass_fbo->UnBind(m_lightpass_cmdbuffer);

		// finish recording
		DebugLog::EC(vkEndCommandBuffer(m_lightpass_cmdbuffer));
	}

	void luna::Renderer::RecordFinalFrame_()
	{	
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		// can record the command buffer liao
		// these command buffer is for the final rendering and final presentation on the screen 
		for (size_t i = 0; i < m_finalpass_cmdbuffers.size(); ++i)
		{
			const VkCommandBuffer& commandbuffer = m_finalpass_cmdbuffers[i];
			FinalFBO* finalfbo = m_finalpass_fbos[i];

			vkBeginCommandBuffer(commandbuffer, &beginInfo);

			// starting a render pass 
			// bind the fbo
			finalfbo->Bind(commandbuffer);

			// bind the shader pipeline stuff
			m_finalpass_shader->Bind(commandbuffer);
			m_finalpass_shader->SetViewPort(commandbuffer, finalfbo->getResolution());

			// Draw the quad with final images
			ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(commandbuffer);

			// bind the ui related shaders
			m_text_shader->Bind(commandbuffer);
			m_text_shader->SetViewPort(commandbuffer, finalfbo->getResolution());

			ModelResources::getInstance()->Models[FONT_MODEL]->DrawInstanced(commandbuffer, totaltext);

			// unbind the fbo
			finalfbo->UnBind(commandbuffer);

			// finish recording
			DebugLog::EC(vkEndCommandBuffer(commandbuffer));
		}
	}

	void Renderer::Record()
	{
		RecordMRTOffscreen_();
		RecordLightPassOffscreen_();
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
		submitInfo.pSignalSemaphores = &m_mrtComplete; // will tell the next cmdbuffer, when i render finish
		submitInfo.pCommandBuffers = &m_mrt_cmdbuffer;
		vkQueueSubmit(m_graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);
		
		// lightpass rendering job submit
		submitInfo.pWaitSemaphores = &m_mrtComplete; // wait for mrt present finish
		submitInfo.pSignalSemaphores = &m_lightpassComplete; // will tell the next cmdbuffer, when i render finish
		submitInfo.pCommandBuffers = &m_lightpass_cmdbuffer;
		vkQueueSubmit(m_graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);

		// final image submiting after MRT pass
		submitInfo.pWaitSemaphores = &m_lightpassComplete; // wait for someone render finish
		submitInfo.pSignalSemaphores = &m_renderComplete; // will tell the swap chain to present, when i render finish
		submitInfo.pCommandBuffers = &m_finalpass_cmdbuffers[imageindex];
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

		if (m_fontinstance_ssbo != nullptr)
		{
			delete m_fontinstance_ssbo;
			m_fontinstance_ssbo = nullptr;
		}
		
		if (m_mrt_shader != nullptr)
		{
			m_mrt_shader->Destroy();
			delete m_mrt_shader;
			m_mrt_shader = nullptr;
		}

		if (m_dirlightpass_shader != nullptr)
		{
			m_dirlightpass_shader->Destroy();
			delete m_dirlightpass_shader;
			m_dirlightpass_shader = nullptr;
		}

		if (m_finalpass_shader != nullptr)
		{
			m_finalpass_shader->Destroy();
			delete m_finalpass_shader;
			m_finalpass_shader = nullptr;
		}

		if (m_text_shader!= nullptr)
		{
			m_text_shader->Destroy();
			delete m_text_shader;
			m_text_shader = nullptr;
		}

		if (m_mrt_fbo != nullptr)
		{
			m_mrt_fbo->Destroy();
			delete m_mrt_fbo;
			m_mrt_fbo = nullptr;
		}

		if (m_lightpass_fbo != nullptr)
		{
			m_lightpass_fbo->Destroy();
			delete m_lightpass_fbo;
			m_lightpass_fbo = nullptr;
		}

		for (auto &fbo : m_finalpass_fbos)
		{
			if (fbo != nullptr)
			{
				fbo->Destroy();
				delete fbo;
				fbo = nullptr;
			}
		}
		m_finalpass_fbos.clear();

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
		if (m_mrtComplete != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_mrtComplete, nullptr);
			m_mrtComplete = VK_NULL_HANDLE;
		}
		if (m_lightpassComplete != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_lightpassComplete, nullptr);
			m_lightpassComplete = VK_NULL_HANDLE;
		}

		if (m_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_logicaldevice, m_commandPool, nullptr);
			m_commandPool = VK_NULL_HANDLE;
		}
	}
}
