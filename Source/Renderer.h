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

	class DeferredShader;
	class SkyBoxShader;
	class DirLightPassShader;
	class FinalPassShader;
	class TextShader;
	class GausianBlur1DShader;

	class Renderer :
		public VulkanRenderer
	{
	public:
		/* create swapchain, shaders, fbo, renderpass, etc */
		void CreateResources() override;

		/* delete swap chain, fbo, shaders and renderpass, etc */
		void CleanUpResources() override;

		/* update all the necessary datas to the gpu */
		void UploadDatas(UBOData& ubo, std::vector<InstanceData>& instancedatas, std::vector<FontInstanceData>& fontinstancedatas);

		/* record the dynamic geometry pass */
		void RecordGeometryPass(std::vector<RenderingInfo>& renderinfos);

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
		void RecordDeferredOffscreen_();

		/* primary command buffer -> computer shader pass */
		void RecordCompute_();

		/* primary command buffer -> final rendering && presentation pass */
		void RecordPresentation_();

		Renderer();
		virtual ~Renderer() {/*do nothing*/}

	private:
		/* swapchain images for drawing and presenting onto the screen */
		VulkanSwapchain* m_swapchain = nullptr;

		/* fbos for gpu to read/write */
		DeferredFBO* m_deferred_fbo = nullptr;
		std::vector<FinalFBO*> m_finalpass_fbos;

		/* all the shaders */
		DeferredShader* m_deferred_shader = nullptr;
		SkyBoxShader* m_skybox_shader = nullptr;
		DirLightPassShader* m_dirlightpass_shader = nullptr;
		FinalPassShader* m_finalpass_shader = nullptr;
		TextShader* m_text_shader = nullptr;
		GausianBlur1DShader* m_gausianblur_shader = nullptr;

		/* rendering recording purpose */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_finalpass_cmdbuffers;
		VkCommandBuffer m_deferred_cmdbuffer = VK_NULL_HANDLE;
		VkCommandBuffer m_geometry_secondary_cmdbuff;
		
		/* computing recording purpose */
		VkCommandPool m_comp_cmdpool = VK_NULL_HANDLE;
		VkCommandBuffer m_comp_cmdbuffer = VK_NULL_HANDLE;

		// semaphores for synchronizing read/write images in gpu 
		VkSemaphore m_presentComplete = VK_NULL_HANDLE;
		VkSemaphore m_finalpass_renderComplete = VK_NULL_HANDLE;
		VkSemaphore m_deferred_renderComplete = VK_NULL_HANDLE;
		VkSemaphore m_compute_computeComplete = VK_NULL_HANDLE;

		/* a universal UBO */
		UBO* m_ubo = nullptr;

		/* ssbo for instancing data */
		SSBO* m_instance_ssbo = nullptr;
		SSBO* m_fontinstance_ssbo = nullptr;

		static std::once_flag m_sflag;
		static Renderer* m_instance;
	};
}

#endif

