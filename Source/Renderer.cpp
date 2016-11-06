#include "Renderer.h"
#include "DebugLog.h"

#include "VulkanSwapchain.h"
#include "SSBO.h"
#include "UBO.h"

#include "DeferredFBO.h"
#include "FinalFBO.h"
#include "PresentationFBO.h"

#include "DeferredShader.h"
#include "SkyBoxShader.h"
#include "DirLightPassShader.h"
#include "NonLightPassShader.h"
#include "FinalPassShader.h"
#include "SimpleShader.h"
#include "TextShader.h"

#include "ModelResources.h"
#include "Model.h"
#include "TextureResources.h"
#include "VulkanImageBufferObject.h"
#include "enum_c.h"

#include "CommandBufferPacket.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/glm.hpp>

#define BASE_RESOLUTION_X 1600
#define BASE_RESOLUTION_Y 900
#define MAX_INSTANCEDATA 100
#define MAX_FONTDATA 256

namespace luna
{
	std::once_flag Renderer::m_sflag{};
	Renderer* Renderer::m_instance = nullptr;

	Renderer::Renderer()
	{
		// vulkan instance and logical device will be created in the parents constructor

		m_submitInfo[0] = {};
		m_submitInfo[1] = {};
	}

	void Renderer::CreateResources()
	{
		if (m_swapchain == nullptr)
		{
			// all the models resources will be init 
			ModelResources* models = ModelResources::getInstance();

			// all the textures resources will be init
			TextureResources* texrsc = TextureResources::getInstance();

			// ubo and ssbo that are unique to this renderer
			m_ubo = new UBO();
			m_instance_ssbo = new SSBO(MAX_INSTANCEDATA * sizeof(InstanceData)); // 100 different models reserve
			m_fontinstance_ssbo = new SSBO(MAX_FONTDATA * sizeof(FontInstanceData)); // 256 different characters reserve

			// swap chain and final fbo creation
			// create the swap chain for presenting images
			m_swapchain = new VulkanSwapchain();

			// fbos, shaders will be created 
			CreateRenderPassResources_();

			// command pools and command buffers creation
			CreateCommandBuffers_();

			// create semaphores for presetation and rendering synchronisation
			VkSemaphoreCreateInfo semaphore_createInfo{};
			semaphore_createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_presentComplete));
			DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_offscreen_renderComplete));
			DebugLog::EC(vkCreateSemaphore(m_logicaldevice, &semaphore_createInfo, nullptr, &m_presentpass_renderComplete));

			// pre record the fixed primary command buffers
			PreRecord_();
		}
	}

	void Renderer::CreateRenderPassResources_()
	{
		VkClearValue clearvalue{};
		clearvalue.color = {0.f, 1.f, 1.f, 1.f};

		// deferred fbo with 2 subpass
		m_deferred_fbo = new DeferredFBO();
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::COLOR0_ATTACHMENT);
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::COLOR1_ATTACHMENT);
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::HDRCOLOR_ATTACHMENT);
		clearvalue.depthStencil = {1.f, 0xff};
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::DEPTHSTENCIL_ATTACHMENT);
		m_deferred_fbo->Init({BASE_RESOLUTION_X, BASE_RESOLUTION_Y});

		// final pass fbo
		m_final_fbo = new FinalFBO();
		m_final_fbo->Clear(clearvalue, FINAL_FBOATTs::COLOR_ATTACHMENT);
		m_final_fbo->Init({BASE_RESOLUTION_X, BASE_RESOLUTION_Y});

		// create framebuffer for each swapchain images
		clearvalue.color = {0.f, 1.f, 0.f, 1.f};
		m_presentation_fbos.resize(m_swapchain->getImageCount());
		for (int i = 0; i < m_presentation_fbos.size(); i++)
		{
			m_presentation_fbos[i] = new PresentationFBO();
			m_presentation_fbos[i]->SetAttachment(
				m_swapchain->m_buffers[i].image, 
				m_swapchain->m_buffers[i].imageview, 
				m_swapchain->getColorFormat(), 
				PRESENT_FBOATTs::COLOR_ATTACHMENT
			);
			m_presentation_fbos[i]->Clear(clearvalue, PRESENT_FBOATTs::COLOR_ATTACHMENT);
			m_presentation_fbos[i]->Init(m_swapchain->getExtent()); // must be the same as swap chain extent
		}

		TextureResources* texrsc = TextureResources::getInstance();
		
		// available textures for the shader descriptors
		std::vector<VulkanImageBufferObject*> totalDiffTex(2);
		totalDiffTex[0] = texrsc->Textures[eTEXTURES::BLACK_2D_RGBA];
		totalDiffTex[1] = texrsc->Textures[eTEXTURES::BASIC_2D_RGBA8];

		// deferred shader init
		m_deferred_shader = new DeferredShader();
		m_deferred_shader->SetDescriptors(m_ubo, m_instance_ssbo, totalDiffTex);
		m_deferred_shader->Init(DeferredFBO::getRenderPass());

		// skybox shader init
		m_skybox_shader = new SkyBoxShader();
		m_skybox_shader->SetDescriptors(m_ubo, texrsc->Textures[eTEXTURES::YOKOHOMO_CUBEMAP_RGBA8]);
		m_skybox_shader->Init(DeferredFBO::getRenderPass());

		// dirlight shader init
		m_dirlightpass_shader = new DirLightPassShader();
		m_dirlightpass_shader->SetDescriptors(
			texrsc->Textures[eTEXTURES::COLOR0_ATTACHMENT_RGBA32U],
			texrsc->Textures[eTEXTURES::COLOR1_ATTACHMENT_RGBA32F]
		);
		m_dirlightpass_shader->Init(DeferredFBO::getRenderPass());

		// a simple non light pass
		m_nonlightpass_shader = new NonLightPassShader();
		m_nonlightpass_shader->SetDescriptors(
			texrsc->Textures[eTEXTURES::COLOR0_ATTACHMENT_RGBA32U]
		);
		m_nonlightpass_shader->Init(DeferredFBO::getRenderPass());

		// final pass shader init
		m_finalpass_shader = new FinalPassShader();
		m_finalpass_shader->SetDescriptors(texrsc->Textures[eTEXTURES::HDRTEX_ATTACHMENT_RGBA16F]);
		m_finalpass_shader->Init(FinalFBO::getRenderPass());

		// simple shader init 
		m_simple_shader = new SimpleShader();
		m_simple_shader->SetDescriptors(texrsc->Textures[eTEXTURES::LDRTEX_ATTACHMENT_RGBA8]);
		m_simple_shader->Init(PresentationFBO::getRenderPass());

		// text shader init
		m_text_shader = new TextShader();
		m_text_shader->SetDescriptors(m_fontinstance_ssbo, texrsc->Textures[eTEXTURES::EVAFONT_2D_BC3]);
		m_text_shader->Init(FinalFBO::getRenderPass());

		// just to double make sure that the shader update the latest size of the ssbo
		m_deferred_shader->UpdateDescriptor(m_instance_ssbo);
		// just to double make sure that the shader update the latest size of the ssbo
		m_text_shader->UpdateDescriptor(m_fontinstance_ssbo); 
	}

	void Renderer::CreateCommandBuffers_()
	{
		VkCommandPoolCreateInfo commandPool_createinfo{};
		commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPool_createinfo.queueFamilyIndex = m_queuefamily_index.graphicsFamily;
		DebugLog::EC(vkCreateCommandPool(m_logicaldevice, &commandPool_createinfo, nullptr, &m_commandPool));

		m_presentation_cmdbuffers.resize(m_presentation_fbos.size());

		// command buffers creation
		VkCommandBufferAllocateInfo buffer_allocateInfo{};
		buffer_allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		buffer_allocateInfo.commandPool = m_commandPool;
		buffer_allocateInfo.commandBufferCount = (uint32_t)m_presentation_cmdbuffers.size();
		buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, m_presentation_cmdbuffers.data()));

		/* each frames */
		commandbufferpacket = new CommandBufferPacket(m_queuefamily_index.graphicsFamily, m_logicaldevice);
	}

	void Renderer::RecordCopyDataToOptimal_Secondary_(const VkCommandBuffer commandbuff)
	{
		// NOTE: I need to rerecord this command buffer if the transfer size exceed the expectation size !!
		DebugLog::EC(vkResetCommandBuffer(commandbuff, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT));

		VkCommandBufferInheritanceInfo inheritanceinfo{};
		inheritanceinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceinfo;

		DebugLog::EC(vkBeginCommandBuffer(commandbuff, &beginInfo));

		m_instance_ssbo->Record(commandbuff);
		m_fontinstance_ssbo->Record(commandbuff);
		m_ubo->Record(commandbuff);

		DebugLog::EC(vkEndCommandBuffer(commandbuff));
	}

	void Renderer::RecordGeometryPass_Secondary_(const VkCommandBuffer commandbuff, const std::vector<RenderingInfo>& renderinfos)
	{
		auto mr = ModelResources::getInstance();

		vkResetCommandBuffer(commandbuff, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferInheritanceInfo inheritanceinfo{};
		inheritanceinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceinfo.framebuffer = m_deferred_fbo->getFramebuffer();
		inheritanceinfo.subpass = 0;
		inheritanceinfo.occlusionQueryEnable = VK_FALSE;
		inheritanceinfo.renderPass = DeferredFBO::getRenderPass();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceinfo;

		// start recording the geometry pass command buffer
		vkBeginCommandBuffer(commandbuff, &beginInfo);

		auto totalrenderinfosize = renderinfos.size();

		// if there are something to render
		if (totalrenderinfosize > 0)
		{
			// bind the deferred shader
			m_deferred_shader->Bind(commandbuff);
			m_deferred_shader->SetViewPort(commandbuff, m_deferred_fbo->getResolution());
			vkCmdSetStencilCompareMask(commandbuff, VK_STENCIL_FACE_FRONT_BIT, 0xff);
			vkCmdSetStencilWriteMask(commandbuff, VK_STENCIL_FACE_FRONT_BIT, 0xff); // if is 0, then stencil is disable
			vkCmdSetStencilReference(commandbuff, VK_STENCIL_FACE_FRONT_BIT, 1); // set stencil value 1

			// record the first renderinfo first
			const auto& ri = renderinfos[0];
			m_deferred_shader->BindTexture(commandbuff, ri.textureID);
			m_deferred_shader->LoadObjectOffset(commandbuff, 0);
			auto offset = ri.instancedatas.size();
			mr->Models[ri.modelID]->DrawInstanced(commandbuff, static_cast<uint32_t>(offset));

			// record the rest renderinfo
			for (int i = 1; i < totalrenderinfosize; ++i)
			{
				const auto& renderinfo = renderinfos[i];

				m_deferred_shader->BindTexture(commandbuff, renderinfo.textureID);
				m_deferred_shader->LoadObjectOffset(commandbuff, static_cast<uint32_t>(offset));
				mr->Models[renderinfo.modelID]->DrawInstanced(commandbuff, static_cast<uint32_t>(renderinfo.instancedatas.size()));

				offset += renderinfo.instancedatas.size();
			}
		}

		// end geometry pass cmd buff
		vkEndCommandBuffer(commandbuff);
	}

	void Renderer::RecordSkybox__Secondary_(const VkCommandBuffer commandbuff)
	{
		vkResetCommandBuffer(commandbuff, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferInheritanceInfo inheritanceinfo{};
		inheritanceinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceinfo.framebuffer = m_deferred_fbo->getFramebuffer();
		inheritanceinfo.subpass = 0;
		inheritanceinfo.occlusionQueryEnable = VK_FALSE;
		inheritanceinfo.renderPass = DeferredFBO::getRenderPass();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceinfo;

		// start recording the geometry pass command buffer
		vkBeginCommandBuffer(commandbuff, &beginInfo);

		// draw skybox last
		m_skybox_shader->Bind(commandbuff);
		m_skybox_shader->SetViewPort(commandbuff, m_deferred_fbo->getResolution());
		vkCmdSetStencilCompareMask(commandbuff, VK_STENCIL_FACE_FRONT_BIT, 0xff);
		vkCmdSetStencilWriteMask(commandbuff, VK_STENCIL_FACE_FRONT_BIT, 0xff); // if is 0, then stencil is disable

		vkCmdSetStencilReference(commandbuff, VK_STENCIL_FACE_FRONT_BIT, 3); // set stencil value, for skybox is 3
		auto t = glm::translate(glm::mat4(), glm::vec3(0.f, 0, 0.f));
		auto s = glm::scale(glm::mat4(), glm::vec3(100.f, 100.f, 100.f));
		auto r = glm::rotate(glm::mat4(),  glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f));
		m_skybox_shader->LoadModel(commandbuff, t * s * r);
		ModelResources::getInstance()->Models[SKYBOX_MODEL]->Draw(commandbuff);

		// end geometry pass cmd buff
		vkEndCommandBuffer(commandbuff);
	}

	void Renderer::RecordUIPass_Secondary_(const VkCommandBuffer commandbuff, const uint32_t & totaltext)
	{
		vkResetCommandBuffer(commandbuff, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferInheritanceInfo inheritanceinfo{};
		inheritanceinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceinfo.framebuffer = m_final_fbo->getFramebuffer();
		inheritanceinfo.subpass = 0;
		inheritanceinfo.occlusionQueryEnable = VK_FALSE;
		inheritanceinfo.renderPass = FinalFBO::getRenderPass();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceinfo;

		// start recording the geometry pass command buffer
		vkBeginCommandBuffer(commandbuff, &beginInfo);

		if (totaltext > 0)
		{
			// bind the ui related shaders
			m_text_shader->Bind(commandbuff);
			m_text_shader->SetViewPort(commandbuff, m_final_fbo->getResolution());
			ModelResources::getInstance()->Models[FONT_MODEL]->DrawInstanced(commandbuff, totaltext);
		}

		vkEndCommandBuffer(commandbuff);
	}

	void Renderer::RecordSecondaryOffscreen_Secondary_(const VkCommandBuffer commandbuff)
	{
		vkResetCommandBuffer(commandbuff, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferInheritanceInfo inheritanceinfo{};
		inheritanceinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
		inheritanceinfo.framebuffer = m_final_fbo->getFramebuffer();
		inheritanceinfo.subpass = 0;
		inheritanceinfo.occlusionQueryEnable = VK_FALSE;
		inheritanceinfo.renderPass = FinalFBO::getRenderPass();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT | VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceinfo;

		// start recording the geometry pass command buffer
		vkBeginCommandBuffer(commandbuff, &beginInfo);

		// bind the shader pipeline stuff
		m_finalpass_shader->Bind(commandbuff);
		m_finalpass_shader->SetViewPort(commandbuff, m_final_fbo->getResolution());
		ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(commandbuff);

		vkEndCommandBuffer(commandbuff);
	}

	void Renderer::PreRecord_()
	{
		// dummy record secondary buffers
		FramePacket temp{};
		std::vector<RenderingInfo> renderinfos;
		// record the command buffers and update the staging buffers
		RecordGeometryPass_Secondary_(
			commandbufferpacket->geometry_secondary_cmdbuff, 
			renderinfos
		);
		RecordUIPass_Secondary_(
			commandbufferpacket->font_secondary_cmdbuff, 
			0
		);
		RecordCopyDataToOptimal_Secondary_(commandbufferpacket->transferdata_secondary_cmdbuff);
		RecordSkybox__Secondary_(commandbufferpacket->skybox_secondary_cmdbuff);
		RecordSecondaryOffscreen_Secondary_(commandbufferpacket->offscreen_secondary_cmdbuff);

		// primary command buffers, record once and for all
		RecordOffscreen_Primary_(commandbufferpacket->offscreen_cmdbuffer);
		RecordPresentation_Primary_();

		auto& submitinfo0 = m_submitInfo[0];
		submitinfo0.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitinfo0.commandBufferCount = 1;
		submitinfo0.signalSemaphoreCount = 1;
		submitinfo0.waitSemaphoreCount = 1;
		submitinfo0.pWaitDstStageMask = &m_waitStages[0]; // wait until draw finish
		submitinfo0.pWaitSemaphores = &m_presentComplete; 
		submitinfo0.pSignalSemaphores = &m_offscreen_renderComplete; // will inform the next one, when i render finish
		submitinfo0.pCommandBuffers = &commandbufferpacket->offscreen_cmdbuffer;

		auto& submitinfo1 = m_submitInfo[1];
		submitinfo1.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitinfo1.commandBufferCount = 1;
		submitinfo1.signalSemaphoreCount = 1;
		submitinfo1.waitSemaphoreCount = 1;
		submitinfo1.pWaitDstStageMask = &m_waitStages[0]; // wait until draw finish
		submitinfo1.pWaitSemaphores = &m_offscreen_renderComplete; // wait for someone render finish
		submitinfo1.pSignalSemaphores = &m_presentpass_renderComplete; // will tell the swap chain to present, when i render finish
	}

	void luna::Renderer::RecordOffscreen_Primary_(const VkCommandBuffer commandbuff)
	{
		// begin init
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		// start recording the offscreen command buffers
		DebugLog::EC(vkBeginCommandBuffer(commandbuff, &beginInfo));

		// transfer data to gpu first
		vkCmdExecuteCommands(commandbuff, 1, &commandbufferpacket->transferdata_secondary_cmdbuff);

		// bind the fbo
		m_deferred_fbo->Bind(commandbuff, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		// execute the geometry pass
		VkCommandBuffer secondary_cmdbuff[] = {
			commandbufferpacket->geometry_secondary_cmdbuff, 
			commandbufferpacket->skybox_secondary_cmdbuff
		};
		vkCmdExecuteCommands(commandbuff, 2, secondary_cmdbuff);
		
		// next subpass for lighting calculation
		vkCmdNextSubpass(
			commandbuff, 
			VK_SUBPASS_CONTENTS_INLINE
		);

		// execute all the light passes
		// bind the dirlight pass shader
		m_dirlightpass_shader->Bind(commandbuff);
		m_dirlightpass_shader->SetViewPort(commandbuff, m_deferred_fbo->getResolution());
		vkCmdSetStencilCompareMask(commandbuff, VK_STENCIL_FACE_FRONT_BIT, 0xff);
		vkCmdSetStencilWriteMask(commandbuff, VK_STENCIL_FACE_FRONT_BIT, 0); // if is 0, then stencil is disable
		vkCmdSetStencilReference(commandbuff, VK_STENCIL_FACE_FRONT_BIT, 2); // stencil value to compare with
		ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(commandbuff);

		// combined with those not lightpass geometry
		m_nonlightpass_shader->Bind(commandbuff);
		m_nonlightpass_shader->SetViewPort(commandbuff, m_deferred_fbo->getResolution());
		ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(commandbuff);

		// unbind the fbo
		m_deferred_fbo->UnBind(commandbuff);

		// another render pass
		m_final_fbo->Bind(commandbuff, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		// execute the secondary command buffers
		VkCommandBuffer secondarycommandbuffer[] = { 
			commandbufferpacket->offscreen_secondary_cmdbuff, 
			commandbufferpacket->font_secondary_cmdbuff 
		};
		vkCmdExecuteCommands(commandbuff, 2, secondarycommandbuffer);

		// unbind the fbo
		m_final_fbo->UnBind(commandbuff);

		// finish recording
		DebugLog::EC(vkEndCommandBuffer(commandbuff));
	}

	void luna::Renderer::RecordPresentation_Primary_()
	{	
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		// can record the command buffer liao
		// these command buffer is for the final rendering and final presentation on the screen 
		for (size_t i = 0; i < m_presentation_cmdbuffers.size(); ++i)
		{
			const VkCommandBuffer& commandbuffer = m_presentation_cmdbuffers[i];
			PresentationFBO* presentfbo = m_presentation_fbos[i];

			vkBeginCommandBuffer(commandbuffer, &beginInfo);

			// starting a render pass 
			// bind the fbo
			presentfbo->Bind(commandbuffer);

			// draw a quad to present on screen
			m_simple_shader->Bind(commandbuffer);
			m_simple_shader->SetViewPort(commandbuffer, presentfbo->getResolution());
			ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(commandbuffer);

			// unbind the fbo
			presentfbo->UnBind(commandbuffer);

			// finish recording
			DebugLog::EC(vkEndCommandBuffer(commandbuffer));
		}
	}

	void Renderer::RecordBuffers(const FramePacket & framepacket, std::array<Worker*, 2>& workers)
	{
		workers[0]->addJob([&]() {
			// record the command buffers and update the staging buffers
			RecordGeometryPass_Secondary_(
				commandbufferpacket->geometry_secondary_cmdbuff,
				*framepacket.renderinfos
			);
		});
		
		workers[1]->addJob([&]() {

			// update to staging buffers
			m_instance_ssbo->Update(framepacket.instancedatas);
			m_fontinstance_ssbo->Update(framepacket.fontinstancedatas);
			m_ubo->Update(framepacket.maincamdata);

			RecordUIPass_Secondary_(
				commandbufferpacket->font_secondary_cmdbuff,
				static_cast<uint32_t>(framepacket.fontinstancedatas.size())
			);
		});

		// let all workers finish their job pls
		workers[0]->wait();
		workers[1]->wait();
	}

	void Renderer::RecreateSwapChain()
	{
		vkDeviceWaitIdle(m_logicaldevice);

		m_swapchain->RecreateSwapChain();

		// destroy the presentation fbos and then create them again
		for (auto &fbo : m_presentation_fbos)
		{
			fbo->Destroy();
		}

		// create framebuffer for each swapchain images
		VkClearValue clearvalue{};
		clearvalue.color = {0.f, 1.f, 0.f, 1.f};
		for (int i = 0; i < m_presentation_fbos.size(); i++)
		{
			m_presentation_fbos[i]->SetAttachment(
				m_swapchain->m_buffers[i].image, 
				m_swapchain->m_buffers[i].imageview, 
				m_swapchain->getColorFormat(), 
				PRESENT_FBOATTs::COLOR_ATTACHMENT
			);
			m_presentation_fbos[i]->Clear(clearvalue, PRESENT_FBOATTs::COLOR_ATTACHMENT);
			m_presentation_fbos[i]->Init(m_swapchain->getExtent()); // must be the same as swap chain extent
		}

		// recreate the command buffers again
		if (m_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_logicaldevice, m_commandPool, nullptr);
			m_commandPool = VK_NULL_HANDLE;
		}
		m_presentation_cmdbuffers.clear();

		VkCommandPoolCreateInfo commandPool_createinfo{};
		commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPool_createinfo.queueFamilyIndex = m_queuefamily_index.graphicsFamily;
		DebugLog::EC(vkCreateCommandPool(m_logicaldevice, &commandPool_createinfo, nullptr, &m_commandPool));

		m_presentation_cmdbuffers.resize(m_presentation_fbos.size());

		// command buffers creation
		VkCommandBufferAllocateInfo buffer_allocateInfo{};
		buffer_allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		buffer_allocateInfo.commandPool = m_commandPool;
		buffer_allocateInfo.commandBufferCount = (uint32_t)m_presentation_cmdbuffers.size();
		buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, m_presentation_cmdbuffers.data()));

		// then record it again
		RecordPresentation_Primary_();
	}

	void Renderer::Render()
	{
		// get the available image to render with
		uint32_t imageindex = 0;

		DebugLog::EC(m_swapchain->AcquireNextImage(m_presentComplete, &imageindex));

		m_submitInfo[1].pCommandBuffers = &m_presentation_cmdbuffers[imageindex];

		// submit all the queues
		DebugLog::EC(vkQueueSubmit(m_graphic_queue, 2, m_submitInfo, VK_NULL_HANDLE));

		// present it on the screen pls
		DebugLog::EC(m_swapchain->QueuePresent(m_graphic_queue, imageindex, m_presentpass_renderComplete));
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

		if (m_skybox_shader != nullptr)
		{
			m_skybox_shader->Destroy();
			delete m_skybox_shader;
			m_skybox_shader = nullptr;
		}

		if (m_dirlightpass_shader != nullptr)
		{
			m_dirlightpass_shader->Destroy();
			delete m_dirlightpass_shader;
			m_dirlightpass_shader = nullptr;
		}

		if (m_nonlightpass_shader != nullptr)
		{
			m_nonlightpass_shader->Destroy();
			delete m_nonlightpass_shader;
			m_nonlightpass_shader = nullptr;
		}

		if (m_finalpass_shader != nullptr)
		{
			m_finalpass_shader->Destroy();
			delete m_finalpass_shader;
			m_finalpass_shader = nullptr;
		}

		if (m_simple_shader != nullptr)
		{
			m_simple_shader->Destroy();
			delete m_simple_shader;
			m_simple_shader = nullptr;
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

		if (m_final_fbo != nullptr)
		{
			m_final_fbo->Destroy();
			delete m_final_fbo;
			m_final_fbo = nullptr;
		}

		for (auto &fbo : m_presentation_fbos)
		{
			if (fbo != nullptr)
			{
				fbo->Destroy();
				delete fbo;
				fbo = nullptr;
			}
		}
		m_presentation_fbos.clear();

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
		if (m_presentpass_renderComplete != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_presentpass_renderComplete, nullptr);
			m_presentpass_renderComplete = VK_NULL_HANDLE;
		}
		if (m_offscreen_renderComplete != VK_NULL_HANDLE)
		{
			vkDestroySemaphore(m_logicaldevice, m_offscreen_renderComplete, nullptr);
			m_offscreen_renderComplete = VK_NULL_HANDLE;
		}
		
		if (m_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_logicaldevice, m_commandPool, nullptr);
			m_commandPool = VK_NULL_HANDLE;
		}
		
		if (commandbufferpacket != nullptr)
		{
			commandbufferpacket->Destroy(m_logicaldevice);
			commandbufferpacket = nullptr;
		}
	}
}

/*
void Renderer::RecordCompute_()
{
// begin init

// will blit the HDR image to smaller texture .. for bluring later
VkImageSubresourceLayers srcsubrsc{};
srcsubrsc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
srcsubrsc.baseArrayLayer = 0;
srcsubrsc.mipLevel = 0; // at miplevel 0
srcsubrsc.layerCount = 1; // only got one mipmap

VkImageSubresourceLayers dstsubrsc{};
dstsubrsc.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
dstsubrsc.baseArrayLayer = 0;
dstsubrsc.mipLevel = 0; // at miplevel 0
dstsubrsc.layerCount = 1; // only got one mipmap

VulkanImageBufferObject* srcimg = TextureResources::getInstance()->Textures[HDRTEX_ATTACHMENT_RGBA16F];
VulkanImageBufferObject* dstimg = TextureResources::getInstance()->Textures[HORBLUR_2D_RGBA16F];

VkImageBlit region{};
region.srcOffsets[1].x = srcimg->getWidth();
region.srcOffsets[1].y = srcimg->getHeight();
region.srcOffsets[1].z = 1;
region.srcSubresource = srcsubrsc;

region.dstOffsets[1].x = dstimg->getWidth();
region.dstOffsets[1].y = dstimg->getHeight();
region.dstOffsets[1].z = 1;
region.dstSubresource = dstsubrsc;

VkCommandBufferBeginInfo beginInfo{};
beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
beginInfo.pInheritanceInfo = nullptr;

DebugLog::EC(vkBeginCommandBuffer(m_comp_cmdbuffer, &beginInfo));

VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
m_comp_cmdbuffer, srcimg->getImage(),
VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT,
VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
);

VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
m_comp_cmdbuffer, dstimg->getImage(),
VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
VK_ACCESS_SHADER_READ_BIT, VK_ACCESS_TRANSFER_WRITE_BIT,
VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
);

vkCmdBlitImage(
m_comp_cmdbuffer,
srcimg->getImage(),
VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
dstimg->getImage(),
VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
1,
&region,
VK_FILTER_LINEAR
);

// change back to optimal layout
VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
m_comp_cmdbuffer, srcimg->getImage(),
VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_SHADER_READ_BIT,
VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
);

VulkanImageBufferObject::TransitionAttachmentImagesLayout_(
m_comp_cmdbuffer, dstimg->getImage(),
VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT
);

auto resolution = glm::ivec2(dstimg->getWidth(), dstimg->getHeight());
auto workgroup = glm::ivec2(resolution.x / 16, resolution.y / 16);

// start compute blur 
m_gausianblur_shader->Bind(m_comp_cmdbuffer);
m_gausianblur_shader->LoadResolution(m_comp_cmdbuffer, glm::ivec2(resolution.x, resolution.y));

for (int i = 0; i < 16; ++i)
{
// verticle pass
m_gausianblur_shader->BindDescriptorSet(m_comp_cmdbuffer, 0);
m_gausianblur_shader->LoadBlurDirection(m_comp_cmdbuffer, glm::ivec2(0, 1));
vkCmdDispatch(m_comp_cmdbuffer, workgroup.x, workgroup.y, 1);

// horizontal pass again
m_gausianblur_shader->BindDescriptorSet(m_comp_cmdbuffer, 1);
m_gausianblur_shader->LoadBlurDirection(m_comp_cmdbuffer, glm::ivec2(1, 0));
vkCmdDispatch(m_comp_cmdbuffer, workgroup.x, workgroup.y, 1);
}

DebugLog::EC(vkEndCommandBuffer(m_comp_cmdbuffer));
}*/