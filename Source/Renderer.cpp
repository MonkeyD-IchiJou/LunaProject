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
#include "FinalPassShader.h"
#include "SimpleShader.h"
#include "TextShader.h"

#include "ModelResources.h"
#include "Model.h"
#include "TextureResources.h"
#include "VulkanImageBufferObject.h"
#include "enum_c.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm\glm.hpp>

#define BASE_RESOLUTION_X 1280
#define BASE_RESOLUTION_Y 720
#define MAX_INSTANCEDATA 100
#define MAX_FONTDATA 256

namespace luna
{
	std::once_flag Renderer::m_sflag{};
	Renderer* Renderer::m_instance = nullptr;

	Renderer::Renderer()
	{
		// vulkan instance and logical device will be created in the parents constructor
	}

	void Renderer::MapGeometryDatas(const std::vector<InstanceData>& instancedatas)
	{
		m_instance_ssbo->Update(instancedatas);
	}

	void Renderer::MapFontInstDatas(const std::vector<FontInstanceData>& fontinstancedatas)
	{
		m_fontinstance_ssbo->Update(fontinstancedatas);
	}

	void Renderer::MapMainCamDatas(const UBOData & ubo)
	{
		m_ubo->Update(ubo);
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
			m_swapchain->Init();

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
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::WORLDPOS_ATTACHMENT);
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::WORLDNORM_ATTACHMENT);
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::ALBEDO_ATTACHMENT);
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::HDRCOLOR_ATTACHMENT);
		clearvalue.depthStencil = {1.f, 0xff};
		m_deferred_fbo->Clear(clearvalue, DFR_FBOATTs::DEPTHSTENCIL_ATTACHMENT);
		m_deferred_fbo->Init({BASE_RESOLUTION_X, BASE_RESOLUTION_Y});

		// final pass fbo
		m_final_fbo = new FinalFBO();
		m_final_fbo->Clear(clearvalue, FINAL_FBOATTs::COLOR_ATTACHMENT);
		m_final_fbo->Init({BASE_RESOLUTION_X, BASE_RESOLUTION_Y});

		// create framebuffer for each swapchain images
		clearvalue.color = {0.f, 0.f, 0.f, 1.f};
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
		std::vector<VulkanImageBufferObject*> totalDiffTex(3);
		totalDiffTex[0] = texrsc->Textures[eTEXTURES::BLACK_2D_RGBA];
		totalDiffTex[1] = texrsc->Textures[eTEXTURES::BASIC_2D_BC2];
		totalDiffTex[2] = texrsc->Textures[eTEXTURES::BASIC_2D_RGBA8];

		// deferred shader init
		m_deferred_shader = new DeferredShader();
		m_deferred_shader->SetDescriptors(m_ubo, m_instance_ssbo, totalDiffTex);
		m_deferred_shader->Init(DeferredFBO::getRenderPass());

		// skybox shader init
		m_skybox_shader = new SkyBoxShader();
		m_skybox_shader->SetDescriptors(m_ubo, texrsc->Textures[eTEXTURES::YOKOHOMO_CUBEMAP_BC3]);
		m_skybox_shader->Init(DeferredFBO::getRenderPass());

		// dirlight shader init
		m_dirlightpass_shader = new DirLightPassShader();
		m_dirlightpass_shader->SetDescriptors(
			texrsc->Textures[eTEXTURES::WORLDPOS_ATTACHMENT_RGBA16F],
			texrsc->Textures[eTEXTURES::WORLDNORM_ATTACHMENT_RGBA16F],
			texrsc->Textures[eTEXTURES::ALBEDO_ATTACHMENT_RGBA16F]
		);
		m_dirlightpass_shader->Init(DeferredFBO::getRenderPass());

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
	}

	void Renderer::CreateCommandBuffers_()
	{
		// create the command pool first
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
			buffer_allocateInfo.commandBufferCount = 1;
			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_deferred_cmdbuffer));
			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_finalpass_cmdbuffer));
		}
		
		{
			// secondary command buffer creation
			// it is able to reset the command buffer
			VkCommandPoolCreateInfo commandPool_createinfo{};
			commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPool_createinfo.queueFamilyIndex = m_queuefamily_index.graphicsFamily;
			commandPool_createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			DebugLog::EC(vkCreateCommandPool(m_logicaldevice, &commandPool_createinfo, nullptr, &m_secondary_commandPool));

			VkCommandBufferAllocateInfo buffer_allocateInfo{};
			buffer_allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			buffer_allocateInfo.commandPool = m_secondary_commandPool;
			buffer_allocateInfo.commandBufferCount = 1;
			buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_geometry_secondary_cmdbuff));
			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_font_secondary_cmdbuff));
			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_offscreen_secondary_cmdbuff));
			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_skybox_secondary_cmdbuff));
			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_transferdata_secondary_cmdbuff));
		}
	}

	void Renderer::RecordTransferData_Secondary()
	{
		// NOTE: I need to rerecord this command buffer if the transfer size exceed the expectation size !!
		vkResetCommandBuffer(m_transferdata_secondary_cmdbuff, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		VkCommandBufferInheritanceInfo inheritanceinfo{};
		inheritanceinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = &inheritanceinfo;

		DebugLog::EC(vkBeginCommandBuffer(m_transferdata_secondary_cmdbuff, &beginInfo));

		m_instance_ssbo->Record(m_transferdata_secondary_cmdbuff);
		m_fontinstance_ssbo->Record(m_transferdata_secondary_cmdbuff);
		m_ubo->Record(m_transferdata_secondary_cmdbuff);

		DebugLog::EC(vkEndCommandBuffer(m_transferdata_secondary_cmdbuff));
	}

	void Renderer::RecordGeometryPass_Secondary(const std::vector<RenderingInfo>& renderinfos)
	{
		auto mr = ModelResources::getInstance();

		vkResetCommandBuffer(m_geometry_secondary_cmdbuff, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

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

		// just to double make sure that the shader update the latest size of the ssbo
		m_deferred_shader->UpdateDescriptor(m_instance_ssbo);

		// start recording the geometry pass command buffer
		vkBeginCommandBuffer(m_geometry_secondary_cmdbuff, &beginInfo);

		auto totalrenderinfosize = renderinfos.size();

		// if there are something to render
		if (totalrenderinfosize > 0)
		{
			// bind the deferred shader
			m_deferred_shader->Bind(m_geometry_secondary_cmdbuff);
			m_deferred_shader->SetViewPort(m_geometry_secondary_cmdbuff, m_deferred_fbo->getResolution());
			vkCmdSetStencilCompareMask(m_geometry_secondary_cmdbuff, VK_STENCIL_FACE_FRONT_BIT, 0xff);
			vkCmdSetStencilWriteMask(m_geometry_secondary_cmdbuff, VK_STENCIL_FACE_FRONT_BIT, 0xff); // if is 0, then stencil is disable
			vkCmdSetStencilReference(m_geometry_secondary_cmdbuff, VK_STENCIL_FACE_FRONT_BIT, 1); // set stencil value

			// record the first renderinfo first
			const auto& ri = renderinfos[0];
			m_deferred_shader->BindTexture(m_geometry_secondary_cmdbuff, ri.textureID);
			m_deferred_shader->LoadObjectOffset(m_geometry_secondary_cmdbuff, 0);
			auto offset = ri.instancedatas.size();
			mr->Models[ri.modelID]->DrawInstanced(m_geometry_secondary_cmdbuff, static_cast<uint32_t>(offset));

			// record the rest renderinfo
			for (int i = 1; i < totalrenderinfosize; ++i)
			{
				const auto& renderinfo = renderinfos[i];

				m_deferred_shader->BindTexture(m_geometry_secondary_cmdbuff, renderinfo.textureID);
				m_deferred_shader->LoadObjectOffset(m_geometry_secondary_cmdbuff, static_cast<uint32_t>(offset));
				mr->Models[renderinfo.modelID]->DrawInstanced(m_geometry_secondary_cmdbuff, static_cast<uint32_t>(renderinfo.instancedatas.size()));

				offset += renderinfo.instancedatas.size();
			}
		}

		// end geometry pass cmd buff
		vkEndCommandBuffer(m_geometry_secondary_cmdbuff);
	}

	void Renderer::RecordSkybox__Secondary_()
	{
		vkResetCommandBuffer(m_skybox_secondary_cmdbuff, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

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
		vkBeginCommandBuffer(m_skybox_secondary_cmdbuff, &beginInfo);

		// draw skybox last
		m_skybox_shader->Bind(m_skybox_secondary_cmdbuff);
		m_skybox_shader->SetViewPort(m_skybox_secondary_cmdbuff, m_deferred_fbo->getResolution());
		vkCmdSetStencilCompareMask(m_skybox_secondary_cmdbuff, VK_STENCIL_FACE_FRONT_BIT, 0xff);
		vkCmdSetStencilWriteMask(m_skybox_secondary_cmdbuff, VK_STENCIL_FACE_FRONT_BIT, 0xff); // if is 0, then stencil is disable

		vkCmdSetStencilReference(m_skybox_secondary_cmdbuff, VK_STENCIL_FACE_FRONT_BIT, 3); // set stencil value, for skybox is 3
		auto t = glm::translate(glm::mat4(), glm::vec3(0.f, 0, 0.f));
		auto s = glm::scale(glm::mat4(), glm::vec3(5.f, 5.f, 5.f));
		auto r = glm::rotate(glm::mat4(),  glm::radians(180.f), glm::vec3(0.f, 0.f, 1.f));
		m_skybox_shader->LoadModel(m_skybox_secondary_cmdbuff, t * s * r);
		ModelResources::getInstance()->Models[SKYBOX_MODEL]->Draw(m_skybox_secondary_cmdbuff);

		// end geometry pass cmd buff
		vkEndCommandBuffer(m_skybox_secondary_cmdbuff);
	}

	void Renderer::RecordUIPass_Secondary(const uint32_t & totaltext)
	{
		vkResetCommandBuffer(m_font_secondary_cmdbuff, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

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

		// just to double make sure that the shader update the latest size of the ssbo
		m_text_shader->UpdateDescriptor(m_fontinstance_ssbo); 

		// start recording the geometry pass command buffer
		vkBeginCommandBuffer(m_font_secondary_cmdbuff, &beginInfo);

		if (totaltext > 0)
		{
			// bind the ui related shaders
			m_text_shader->Bind(m_font_secondary_cmdbuff);
			m_text_shader->SetViewPort(m_font_secondary_cmdbuff, m_final_fbo->getResolution());
			ModelResources::getInstance()->Models[FONT_MODEL]->DrawInstanced(m_font_secondary_cmdbuff, totaltext);
		}

		vkEndCommandBuffer(m_font_secondary_cmdbuff);
	}

	void Renderer::RecordSecondaryOffscreen__Secondary_()
	{
		vkResetCommandBuffer(m_offscreen_secondary_cmdbuff, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

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
		vkBeginCommandBuffer(m_offscreen_secondary_cmdbuff, &beginInfo);

		// bind the shader pipeline stuff
		m_finalpass_shader->Bind(m_offscreen_secondary_cmdbuff);
		m_finalpass_shader->SetViewPort(m_offscreen_secondary_cmdbuff, m_final_fbo->getResolution());
		ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(m_offscreen_secondary_cmdbuff);

		vkEndCommandBuffer(m_offscreen_secondary_cmdbuff);
	}

	void Renderer::PreRecord_()
	{
		// dummy record secondary buffers
		std::vector<RenderingInfo> temp;
		RecordTransferData_Secondary();
		RecordGeometryPass_Secondary(temp);
		RecordSkybox__Secondary_();
		RecordUIPass_Secondary(0);
		RecordSecondaryOffscreen__Secondary_();

		// primary command buffers, record once and for all
		RecordOffscreen_Primary_();
		RecordPresentation_Primary_();
	}

	void luna::Renderer::RecordOffscreen_Primary_()
	{
		// begin init
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		// start recording the offscreen command buffers
		DebugLog::EC(vkBeginCommandBuffer(m_deferred_cmdbuffer, &beginInfo));

		// transfer data to gpu first
		vkCmdExecuteCommands(m_deferred_cmdbuffer, 1, &m_transferdata_secondary_cmdbuff);

		// bind the fbo
		m_deferred_fbo->Bind(m_deferred_cmdbuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		// execute the geometry pass
		VkCommandBuffer secondary_cmdbuff[] = {m_geometry_secondary_cmdbuff, m_skybox_secondary_cmdbuff};
		vkCmdExecuteCommands(m_deferred_cmdbuffer, 2, secondary_cmdbuff);
		
		// next subpass for lighting calculation
		vkCmdNextSubpass(
			m_deferred_cmdbuffer, 
			VK_SUBPASS_CONTENTS_INLINE
		);

		// execute all the light passes

		// bind the dirlight pass shader
		m_dirlightpass_shader->Bind(m_deferred_cmdbuffer);
		m_dirlightpass_shader->SetViewPort(m_deferred_cmdbuffer, m_deferred_fbo->getResolution());
		vkCmdSetStencilCompareMask(m_deferred_cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, 0xff);
		vkCmdSetStencilWriteMask(m_deferred_cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, 0); // if is 0, then stencil is disable
		vkCmdSetStencilReference(m_deferred_cmdbuffer, VK_STENCIL_FACE_FRONT_BIT, 3); // stencil value to compare with
		ModelResources::getInstance()->Models[QUAD_MODEL]->Draw(m_deferred_cmdbuffer);

		// combined with those not lightpass geometry

		// unbind the fbo
		m_deferred_fbo->UnBind(m_deferred_cmdbuffer);

		// another render pass
		m_final_fbo->Bind(m_deferred_cmdbuffer, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		// execute the secondary command buffers
		VkCommandBuffer secondarycommandbuffer[] = { m_offscreen_secondary_cmdbuff, m_font_secondary_cmdbuff };
		vkCmdExecuteCommands(m_deferred_cmdbuffer, 2, secondarycommandbuffer);

		// unbind the fbo
		m_final_fbo->UnBind(m_deferred_cmdbuffer);

		// finish recording
		DebugLog::EC(vkEndCommandBuffer(m_deferred_cmdbuffer));
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

	void Renderer::Render()
	{
		// get the available image to render with
		uint32_t imageindex = 0;
		DebugLog::EC(m_swapchain->AcquireNextImage(m_presentComplete, &imageindex));

		VkSubmitInfo submitInfo[2] = {};
		submitInfo[0].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo[0].commandBufferCount = 1;
		submitInfo[0].signalSemaphoreCount = 1;
		submitInfo[0].waitSemaphoreCount = 1;
		submitInfo[0].pWaitDstStageMask = &m_waitStages[0]; // wait until draw finish
		submitInfo[0].pWaitSemaphores = &m_presentComplete; 
		submitInfo[0].pSignalSemaphores = &m_offscreen_renderComplete; // will inform the next one, when i render finish
		submitInfo[0].pCommandBuffers = &m_deferred_cmdbuffer;

		submitInfo[1].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo[1].commandBufferCount = 1;
		submitInfo[1].signalSemaphoreCount = 1;
		submitInfo[1].waitSemaphoreCount = 1;
		submitInfo[1].pWaitDstStageMask = &m_waitStages[0]; // wait until draw finish
		submitInfo[1].pWaitSemaphores = &m_offscreen_renderComplete; // wait for someone render finish
		submitInfo[1].pSignalSemaphores = &m_presentpass_renderComplete; // will tell the swap chain to present, when i render finish
		submitInfo[1].pCommandBuffers = &m_presentation_cmdbuffers[imageindex];

		// submit all the queues
		DebugLog::EC(vkQueueSubmit(m_graphic_queue, 2, submitInfo, VK_NULL_HANDLE));

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
		
		if (m_secondary_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_logicaldevice, m_secondary_commandPool, nullptr);
			m_secondary_commandPool = VK_NULL_HANDLE;
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