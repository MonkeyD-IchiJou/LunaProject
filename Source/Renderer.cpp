#include "Renderer.h"
#include "DebugLog.h"
#include <array>

#include "VulkanSwapchain.h"
#include "SSBO.h"
#include "UBO.h"

#include "DeferredFBO.h"
#include "FinalFBO.h"

#include "DeferredShader.h"
#include "DirLightPassShader.h"
#include "FinalPassShader.h"
#include "TextShader.h"

#include "ModelResources.h"
#include "Model.h"
#include "TextureResources.h"
#include "VulkanImageBufferObject.h"
#include "Font.h"

#include "WinNative.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm\glm.hpp>

#define INSTANCE_COUNT 3
static int32_t totaltext = 0;

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

			// ubo and ssbo that are unique to this renderer
			m_ubo = new UBO();
			m_instance_ssbo = new SSBO(100 * sizeof(InstanceData)); // 100 different models reserve
			m_fontinstance_ssbo = new SSBO(256 * sizeof(FontInstanceData)); // 256 different characters reserve

			/* initial update for ssbo instancedata */
			std::vector<InstanceData> instancedata(INSTANCE_COUNT);
			instancedata[0].model = glm::rotate(glm::mat4(), 45.f, glm::vec3(0, 1.f, 0)); 
			instancedata[0].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[0].model));
			instancedata[1].model = glm::translate(glm::mat4(), glm::vec3(0, 2.f, 0)); 
			instancedata[1].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[1].model));
			instancedata[2].model = glm::translate(glm::mat4(), glm::vec3(0, 0.f, 2.f)); 
			instancedata[2].transpose_inverse_model = glm::transpose(glm::inverse(instancedata[2].model));
			m_instance_ssbo->Update<InstanceData>(instancedata);

			Font* font = new Font();
			font->LoadFonts(getAssetPath() + "Fonts/eva.fnt", 
				(float)texrsc->Textures[eTEXTURES::EVAFONT_2D_BC3]->getWidth(), (float)texrsc->Textures[eTEXTURES::EVAFONT_2D_BC3]->getHeight());

			{
				WinNative* win = WinNative::getInstance();

				glm::mat4 proj = glm::ortho(0.f, (float)win->getWinSizeX(), (float)win->getWinSizeY(), 0.f); // scale with screen size
				glm::mat4 parentT = glm::translate(glm::mat4(), glm::vec3(5.f, 720.f, 0.f));
				glm::mat4 parentR = glm::rotate(glm::mat4(), 0.f, glm::vec3(0, 0, 1.f));
				glm::mat4 parentS = glm::scale(glm::mat4(), glm::vec3(400.f, 400.f, 1.f));

				std::string str = "NEON GENESIS EVANGELION";
				glm::vec2 cursor = {0.f, 0.0f};

				totaltext = static_cast<int32_t>(str.size());
				std::vector<FontInstanceData> fontinstancedata(totaltext);

				for(int i = 0; i < totaltext; ++i)
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

				m_fontinstance_ssbo->Update<FontInstanceData>(fontinstancedata);
			}

			delete font;

			// swap chain and final fbo creation
			// create the swap chain for presenting images
			m_swapchain = new VulkanSwapchain();
			m_swapchain->Init();

			// fbos, shaders will be created 
			CreateRenderPassResources_();

			// command pools and command buffers creation
			CreateCommandBuffers_();

			// create semaphores for presetation and rendering synchronisation
			VkSemaphoreCreateInfo semaphore_createInfo{};
			semaphore_createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_presentComplete));
			DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_finalpass_renderComplete));
			DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_deferred_renderComplete));
		}
	}

	void Renderer::CreateRenderPassResources_()
	{
		VkClearValue clearvalue{};
		clearvalue.color = {0.f, 0.f, 0.f, 1.f};

		// deferred fbo with 2 subpass
		m_deferred_fbo = new DeferredFBO();
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::WORLDPOS_ATTACHMENT);
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::WORLDNORM_ATTACHMENT);
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::ALBEDO_ATTACHMENT);
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::HDRCOLOR_ATTACHMENT);
		clearvalue.depthStencil = {1.f, 0};
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::DEPTH_ATTACHMENT);
		m_deferred_fbo->Init({1920, 1080});

		// create framebuffer for each swapchain images
		clearvalue.color = {0.f, 0.f, 0.f, 1.f};
		m_finalpass_fbos.resize(m_swapchain->getImageCount());
		for (int i = 0; i < m_finalpass_fbos.size(); i++)
		{
			m_finalpass_fbos[i] = new FinalFBO();
			m_finalpass_fbos[i]->SetAttachment(
				m_swapchain->m_buffers[i].image, 
				m_swapchain->m_buffers[i].imageview, 
				m_swapchain->getColorFormat(), 
				FINAL_FBOATTs::COLOR_ATTACHMENT
			);
			m_finalpass_fbos[i]->Clear(clearvalue, FINAL_FBOATTs::COLOR_ATTACHMENT);
			m_finalpass_fbos[i]->Init(m_swapchain->getExtent()); // must be the same as swap chain extent
		}

		TextureResources* texrsc = TextureResources::getInstance();

		// deferred shader init
		m_deferred_shader = new DeferredShader();
		m_deferred_shader->SetDescriptors(m_ubo, m_instance_ssbo, texrsc->getInstance()->Textures[eTEXTURES::BASIC_2D_BC2]);
		m_deferred_shader->Init(DeferredFBO::getRenderPass());

		// dirlight shader init
		m_dirlightpass_shader = new DirLightPassShader();
		m_dirlightpass_shader->SetDescriptors(
			texrsc->getInstance()->Textures[eTEXTURES::WORLDPOS_ATTACHMENT_RGBA16F],
			texrsc->getInstance()->Textures[eTEXTURES::WORLDNORM_ATTACHMENT_RGBA16F],
			texrsc->getInstance()->Textures[eTEXTURES::ALBEDO_ATTACHMENT_RGBA16F]
		);
		m_dirlightpass_shader->Init(DeferredFBO::getRenderPass());

		// final pass shader init
		m_finalpass_shader = new FinalPassShader();
		m_finalpass_shader->SetDescriptors(texrsc->Textures[eTEXTURES::HDRTEX_ATTACHMENT_RGBA16F]);
		m_finalpass_shader->Init(FinalFBO::getRenderPass());

		// text shader init
		m_text_shader = new TextShader();
		m_text_shader->SetDescriptors(m_fontinstance_ssbo, texrsc->Textures[eTEXTURES::EVAFONT_2D_BC3]);
		m_text_shader->Init(FinalFBO::getRenderPass());
	}

	void Renderer::CreateCommandBuffers_()
	{
		// create the command pool first
		VkCommandPoolCreateInfo commandPool_createinfo{};
		commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPool_createinfo.queueFamilyIndex = m_queuefamily_index.graphicsFamily;
		commandPool_createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		DebugLog::EC(vkCreateCommandPool(m_logicaldevice, &commandPool_createinfo, nullptr, &m_commandPool));

		// command buffers creation
		m_finalpass_cmdbuffers.resize(m_finalpass_fbos.size());
		VkCommandBufferAllocateInfo buffer_allocateInfo{};
		buffer_allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		buffer_allocateInfo.commandPool = m_commandPool;
		buffer_allocateInfo.commandBufferCount = (uint32_t)m_finalpass_cmdbuffers.size();
		buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, m_finalpass_cmdbuffers.data()));

		buffer_allocateInfo.commandBufferCount = 1;
		DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_deferred_cmdbuffer));

		//// secondary command buffer
		//buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		//DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_secondary_buffer));
	}

	void luna::Renderer::RecordDeferredOffscreen_()
	{
		// just to double make sure that the shader update the latest size of the ssbo
		m_deferred_shader->UpdateDescriptor(m_instance_ssbo); 

		// begin init
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		// start recording the offscreen command buffers
		vkBeginCommandBuffer(m_deferred_cmdbuffer, &beginInfo);

		// bind the fbo
		m_deferred_fbo->Bind(m_deferred_cmdbuffer);

		// bind the deferred shader
		m_deferred_shader->Bind(m_deferred_cmdbuffer);
		m_deferred_shader->SetViewPort(m_deferred_cmdbuffer, m_deferred_fbo->getResolution());
		vkCmdSetStencilCompareMask(m_deferred_cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, 0xff);
		vkCmdSetStencilWriteMask(m_deferred_cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, 0xff); // if is 0, then stencil is disable

		// draw objects
		m_deferred_shader->LoadObjectOffset(m_deferred_cmdbuffer, 0);
		vkCmdSetStencilReference(m_deferred_cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, 1);
		m_model->DrawInstanced(m_deferred_cmdbuffer, 2);

		m_deferred_shader->LoadObjectOffset(m_deferred_cmdbuffer, 2);
		vkCmdSetStencilReference(m_deferred_cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, 2);
		ModelResources::getInstance()->Models[BUNNY_MODEL]->Draw(m_deferred_cmdbuffer);

		// next subpass for lighting calculation
		vkCmdNextSubpass(
			m_deferred_cmdbuffer, 
			VK_SUBPASS_CONTENTS_INLINE
		);

		// bind the dirlight pass shader
		m_dirlightpass_shader->Bind(m_deferred_cmdbuffer);
		m_dirlightpass_shader->SetViewPort(m_deferred_cmdbuffer, m_deferred_fbo->getResolution());
		vkCmdSetStencilCompareMask(m_deferred_cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, 0xff);
		vkCmdSetStencilWriteMask(m_deferred_cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, 0); // if is 0, then stencil is disable

		// is an integer reference value that is used in the unsigned stencil comparison
		vkCmdSetStencilReference(m_deferred_cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, 2);
		ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(m_deferred_cmdbuffer);

		// unbind the fbo
		m_deferred_fbo->UnBind(m_deferred_cmdbuffer);

		// finish recording
		DebugLog::EC(vkEndCommandBuffer(m_deferred_cmdbuffer));
	}

	void luna::Renderer::RecordFinalFrame_()
	{	
		// just to double make sure that the shader update the latest size of the ssbo
		m_text_shader->UpdateDescriptor(m_fontinstance_ssbo); 

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

	void Renderer::RecordSecondaryCmdbuff_()
	{
		//vkResetCommandBuffer(m_secondary_buffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		//VkCommandBufferInheritanceInfo inheritanceinfo{};
		//inheritanceinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		//inheritanceinfo.framebuffer = m_test_fbo->getFramebuffer();
		//inheritanceinfo.subpass = 0;
		//inheritanceinfo.occlusionQueryEnable = VK_FALSE;
		//inheritanceinfo.renderPass = TestFBO::getRenderPass();

		//VkCommandBufferBeginInfo beginInfo{};
		//beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		//beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		//beginInfo.pInheritanceInfo = &inheritanceinfo;

		//// start recording the second command buffer
		//vkBeginCommandBuffer(m_secondary_buffer, &beginInfo);
		//m_simple_shader->Bind(m_secondary_buffer);
		//m_simple_shader->SetViewPort(m_secondary_buffer, m_test_fbo->getResolution());
		//ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(m_secondary_buffer);
		//vkEndCommandBuffer(m_secondary_buffer);
	}

	void Renderer::Record()
	{
		RecordSecondaryCmdbuff_();
		RecordDeferredOffscreen_();
		RecordFinalFrame_();
	}

	void Renderer::Update()
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

		WinNative* win = WinNative::getInstance();
		UBOData data{};
		data.view = glm::lookAt(glm::vec3(-2.f, 2.f, -5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		data.proj = glm::perspective(glm::radians(45.f), win->getWinSizeX() / static_cast<float>(win->getWinSizeY()), 0.1f, 10.0f); // take note .. hardcoded aspects

		// Vulkan clip space has inverted Y and half Z.
		glm::mat4 Clip = glm::mat4(
			1.0f, 0.0f, 0.0f, 0.0f,
			0.0f, -1.0f, 0.0f, 0.0f,
			0.0f, 0.0f, 0.5f, 0.0f,
			0.0f, 0.0f, 0.5f, 1.0f
		);

		data.proj = Clip * data.proj;

		m_ubo->Update(data);
	}

	void Renderer::Render()
	{
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

		// subpass submit
		submitInfo.pWaitSemaphores = &m_presentComplete; // wait for someone render finish
		submitInfo.pSignalSemaphores = &m_deferred_renderComplete; // will inform the next one, when i render finish
		submitInfo.pCommandBuffers = &m_deferred_cmdbuffer;
		vkQueueSubmit(m_graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);

		// final image submiting after present finish on the screen
		submitInfo.pWaitSemaphores = &m_deferred_renderComplete; // wait for someone render finish
		submitInfo.pSignalSemaphores = &m_finalpass_renderComplete; // will tell the swap chain to present, when i render finish
		submitInfo.pCommandBuffers = &m_finalpass_cmdbuffers[imageindex];
		vkQueueSubmit(m_graphic_queue, 1, &submitInfo, VK_NULL_HANDLE);

		// present it on the screen pls
		m_swapchain->QueuePresent(m_graphic_queue, imageindex, m_finalpass_renderComplete);
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
		
		if (m_deferred_shader != nullptr)
		{
			m_deferred_shader->Destroy();
			delete m_deferred_shader;
			m_deferred_shader = nullptr;
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

		if (m_deferred_fbo != nullptr)
		{
			m_deferred_fbo->Destroy();
			delete m_deferred_fbo;
			m_deferred_fbo = nullptr;
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
		if (m_finalpass_renderComplete != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_finalpass_renderComplete, nullptr);
			m_finalpass_renderComplete = VK_NULL_HANDLE;
		}
		if (m_deferred_renderComplete != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_deferred_renderComplete, nullptr);
			m_deferred_renderComplete = VK_NULL_HANDLE;
		}
		if (m_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_logicaldevice, m_commandPool, nullptr);
			m_commandPool = VK_NULL_HANDLE;
		}
	}
}