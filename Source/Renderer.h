#ifndef RENDERER_H
#define RENDERER_H

#include "VulkanRenderer.h"
#include "FramePacket.h"
#include <mutex>

#include "Worker.h"
#include <array>

namespace luna
{
	class VulkanSwapchain;
	class Model;
	class UBO;
	class SSBO;

	class DeferredFBO;
	class PresentationFBO;

	class GBufferSubpassShader;
	class LightingSubpassShader;
	class SkyBoxShader;
	class CompositeSubpassShader;
	class FinalPassShader;
	class TextShader;

	class CommandBufferPacket;

	class Renderer :
		public VulkanRenderer
	{
	public:
		/* create swapchain, shaders, fbo, renderpass, etc */
		void CreateResources() override;

		/* delete swap chain, fbo, shaders and renderpass, etc */
		void CleanUpResources() override;

		/* tell the gpu what to render */
		/* submit all the queues && render everything && then present it on the screen */
		void RecordAndRender(const FramePacket& framepacket, std::array<Worker*, 2>& workers);

		/* recreate the swap chain when windows resize */
		void RecreateSwapChain();

	public:
		/* Singleton class implementation */
		static inline Renderer* getInstance(void)
		{
			// only called once
			std::call_once(m_sflag, [&]() {
				m_instance = new Renderer();
			});

			return m_instance;
		}

		/* check whether exist or not */
		static inline bool exists(void)
		{
			return m_instance != nullptr;
		}

		/* Warning Once destroyed, forever destroy */
		inline void Destroy() { CleanUpResources(); DeInit_(); }

	private:
		void CreateRenderPassResources_();
		void CreateCommandBuffers_();

		/* pre-record the primary command buffer once and for all */
		void PreRecord_();

		/* primary command buffer -> deffered shading fbo pass */
		void RecordOffscreen_Pri_(const VkCommandBuffer commandbuff);

		/* primary command buffer -> final rendering && presentation pass */
		void RecordPresentation_Pri_();

		/* dynamic secondary command buffers rerecord */
		void RecordCopyDataToOptimal_Sec_(const VkCommandBuffer commandbuff);
		void RecordGBufferSubpass_Sec_(const VkCommandBuffer commandbuff, const std::vector<RenderingInfo>& renderinfos);
		void RecordLightingSubpass_Sec_(const VkCommandBuffer commandbuff, const UBOData& camdata);
		void RecordSkyboxSubpass_Sec_(const VkCommandBuffer commandbuff);
		void RecordFinalComposition_Sec_(const VkCommandBuffer commandbuff, const int& frameindex);
		void RecordUIPass_Sec_(const VkCommandBuffer commandbuff, const uint32_t & totaltext, const int& frameindex);
		
		Renderer();
		virtual ~Renderer() {/*do nothing*/}

	private:
		/* swapchain images for drawing and presenting onto the screen */
		VulkanSwapchain* m_swapchain = nullptr;

		/* fbos for gpu to read/write */
		DeferredFBO* m_deferred_fbo = nullptr;
		std::vector<PresentationFBO*> m_presentation_fbos;

		/* all the shaders */
		GBufferSubpassShader* m_gbuffersubpass_shader = nullptr;
		LightingSubpassShader* m_lightsubpass_shader = nullptr;
		SkyBoxShader* m_skybox_shader = nullptr;
		CompositeSubpassShader* m_composite_shader = nullptr;
		FinalPassShader* m_finalpass_shader = nullptr;
		TextShader* m_text_shader = nullptr;

		/* presentation command buffer recording purpose */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_presentation_cmdbuffers;
		VkCommandPool m_sec_commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_sec_cmdbuffers;
		CommandBufferPacket* commandbufferpacket{};

		struct Presentation_Sec_Cmdbuff
		{
			VkCommandBuffer CompositionCmdbuff = VK_NULL_HANDLE;
			VkCommandBuffer UIPassCmdbuff = VK_NULL_HANDLE;
		};
		std::vector<Presentation_Sec_Cmdbuff>m_presentation_sec_cmdbuff;

		// semaphores for synchronizing read/write images in gpu 
		VkSemaphore m_presentComplete = VK_NULL_HANDLE;
		VkSemaphore m_presentpass_renderComplete = VK_NULL_HANDLE;
		VkSemaphore m_offscreen_renderComplete = VK_NULL_HANDLE;

		/* a universal UBO */
		UBO* m_ubo = nullptr;

		/* a point light ubo */
		UBO* m_ubopointlights = nullptr;

		/* ssbo for instancing data */
		SSBO* m_instance_ssbo = nullptr;
		SSBO* m_fontinstance_ssbo = nullptr;

		// local cache
		VkSubmitInfo m_submitInfo[2];
		VkPipelineStageFlags m_waitStages[2] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };

		static std::once_flag m_sflag;
		static Renderer* m_instance;
	};
}

#endif

