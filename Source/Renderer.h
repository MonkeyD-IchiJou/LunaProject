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
	class FinalFBO;
	class PresentationFBO;

	class DeferredShader;
	class SkyBoxShader;
	class DirLightPassShader;
	class FinalPassShader;
	class SimpleShader;
	class TextShader;
	class NonLightPassShader;

	class CommandBufferPacket;

	class Renderer :
		public VulkanRenderer
	{
	public:
		/* create swapchain, shaders, fbo, renderpass, etc */
		void CreateResources() override;

		/* delete swap chain, fbo, shaders and renderpass, etc */
		void CleanUpResources() override;

		/* submit all the queues && render everything && then present it on the screen */
		void Render();

		/* tell the gpu what to render */
		void RecordBuffers(const FramePacket& framepacket, std::array<Worker*, 2>& workers);

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

		/* primary command buffer -> deffered shader fbo pass */
		void RecordOffscreen_Primary_(const VkCommandBuffer commandbuff);

		/* primary command buffer -> final rendering && presentation pass */
		void RecordPresentation_Primary_();

		/* dynamic secondary command buffers rerecord */
		void RecordCopyDataToOptimal_Secondary_(const VkCommandBuffer commandbuff);
		void RecordGeometryPass_Secondary_(const VkCommandBuffer commandbuff, const std::vector<RenderingInfo>& renderinfos);
		void RecordUIPass_Secondary_(const VkCommandBuffer commandbuff, const uint32_t & totaltext);
		void RecordSkybox__Secondary_(const VkCommandBuffer commandbuff);
		void RecordSecondaryOffscreen_Secondary_(const VkCommandBuffer commandbuff);
		void RecordLightingSubpass_Secondary_(const VkCommandBuffer commandbuff, const UBOData& camdata);

		Renderer();
		virtual ~Renderer() {/*do nothing*/}

	private:
		/* swapchain images for drawing and presenting onto the screen */
		VulkanSwapchain* m_swapchain = nullptr;

		/* fbos for gpu to read/write */
		DeferredFBO* m_deferred_fbo = nullptr;
		FinalFBO* m_final_fbo = nullptr;
		std::vector<PresentationFBO*> m_presentation_fbos;

		/* all the shaders */
		DeferredShader* m_deferred_shader = nullptr;
		SkyBoxShader* m_skybox_shader = nullptr;
		DirLightPassShader* m_dirlightpass_shader = nullptr;
		NonLightPassShader* m_nonlightpass_shader = nullptr;
		FinalPassShader* m_finalpass_shader = nullptr;
		SimpleShader* m_simple_shader = nullptr;
		TextShader* m_text_shader = nullptr;

		/* presentation command buffer recording purpose */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_presentation_cmdbuffers;
		CommandBufferPacket* commandbufferpacket{};

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

