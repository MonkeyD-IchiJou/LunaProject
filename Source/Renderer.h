#ifndef RENDERER_H
#define RENDERER_H

#include "VulkanRenderer.h"
#include <mutex>

namespace luna
{
	class VulkanSwapchain;
	class Model;
	class UBO;
	class SSBO;

	class MrtFBO;
	class LightPassFBO;
	class FinalFBO;
	class TextShader;

	class MRTShader;
	class DirLightPassShader;
	class FinalPassShader;

	class Renderer :
		public VulkanRenderer
	{
	public:
		/* create swapchain, shaders, fbo, renderpass, etc */
		void CreateResources() override;

		/* delete swap chain, fbo, shaders and renderpass, etc */
		void CleanUpResources() override;

		/* record the command buffer for final presentation */
		void Record();

		/* render everything and then present it on the screen */
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
		void RecordMRTOffscreen_();
		void RecordLightPassOffscreen_();
		void RecordFinalFrame_();

		Renderer();
		virtual ~Renderer() {/*do nothing*/}

	private:
		/* swapchain images for drawing and presenting onto the screen */
		VulkanSwapchain* m_swapchain = nullptr;

		/* fbos for gpu to read/write */
		MrtFBO* m_mrt_fbo = nullptr;
		LightPassFBO* m_lightpass_fbo = nullptr;
		std::vector<FinalFBO*> m_finalpass_fbos;

		/* shader to compute the result of pixels */
		MRTShader* m_mrt_shader = nullptr;
		DirLightPassShader* m_dirlightpass_shader = nullptr;
		FinalPassShader* m_finalpass_shader = nullptr;
		TextShader* m_text_shader = nullptr;

		/* recording purpose */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_finalpass_cmdbuffers;
		VkCommandBuffer m_mrt_cmdbuffer = VK_NULL_HANDLE;
		VkCommandBuffer m_lightpass_cmdbuffer = VK_NULL_HANDLE;

		// semaphores for synchronizing read/write images in gpu 
		VkSemaphore m_presentComplete = VK_NULL_HANDLE;
		VkSemaphore m_renderComplete = VK_NULL_HANDLE;
		VkSemaphore m_mrtComplete = VK_NULL_HANDLE;
		VkSemaphore m_lightpassComplete = VK_NULL_HANDLE;

		/* a model with mesh to render */
		Model* m_model = nullptr;

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

