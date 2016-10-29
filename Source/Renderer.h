#ifndef RENDERER_H
#define RENDERER_H

#include "VulkanRenderer.h"
#include "StorageData.h"
#include <mutex>

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

	class Renderer :
		public VulkanRenderer
	{
	public:
		/* create swapchain, shaders, fbo, renderpass, etc */
		void CreateResources() override;

		/* delete swap chain, fbo, shaders and renderpass, etc */
		void CleanUpResources() override;

		/* update all the necessary datas to the host-visible buffer */
		void MapGeometryDatas(const std::vector<InstanceData>& instancedatas);
		void MapFontInstDatas(const std::vector<FontInstanceData>& fontinstancedatas);
		void MapMainCamDatas(const UBOData& ubo);

		/* for transfering datas to the gpu */
		void RecordTransferData_Secondary();

		/* record the dynamic geometry pass */
		void RecordGeometryPass_Secondary(const std::vector<RenderingInfo>& renderinfos);

		/* record the dynamic ui pass */
		void RecordUIPass_Secondary(const uint32_t& totaltext);

		/* submit all the queues && render everything && then present it on the screen */
		void Render();

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
		void RecordOffscreen_Primary_();

		/* secondary command buffer -> skybox pass */
		void RecordSkybox__Secondary_();

		/* secondary command buffer -> final pass setting in secondary buffer */
		void RecordSecondaryOffscreen__Secondary_();

		/* primary command buffer -> final rendering && presentation pass */
		void RecordPresentation_Primary_();

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
		FinalPassShader* m_finalpass_shader = nullptr;
		SimpleShader* m_simple_shader = nullptr;
		TextShader* m_text_shader = nullptr;

		/* rendering recording purpose */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_presentation_cmdbuffers;
		VkCommandBuffer m_offscreen_cmdbuffer = VK_NULL_HANDLE;
		VkCommandBuffer m_finalpass_cmdbuffer = VK_NULL_HANDLE;
		
		/* dynamic recording purpose */
		VkCommandPool m_secondary_commandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_geometry_secondary_cmdbuff = VK_NULL_HANDLE;
		VkCommandBuffer m_font_secondary_cmdbuff = VK_NULL_HANDLE;
		VkCommandBuffer m_offscreen_secondary_cmdbuff = VK_NULL_HANDLE;
		VkCommandBuffer m_skybox_secondary_cmdbuff = VK_NULL_HANDLE;
		VkCommandBuffer m_transferdata_secondary_cmdbuff = VK_NULL_HANDLE;

		// semaphores for synchronizing read/write images in gpu 
		VkSemaphore m_presentComplete = VK_NULL_HANDLE;
		VkSemaphore m_presentpass_renderComplete = VK_NULL_HANDLE;
		VkSemaphore m_offscreen_renderComplete = VK_NULL_HANDLE;

		/* a universal UBO */
		UBO* m_ubo = nullptr;

		/* ssbo for instancing data */
		SSBO* m_instance_ssbo = nullptr;
		SSBO* m_fontinstance_ssbo = nullptr;

		// local cache
		VkPipelineStageFlags m_waitStages[2] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT };

		static std::once_flag m_sflag;
		static Renderer* m_instance;
	};
}

#endif

