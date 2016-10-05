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
	class FinalFBO;

	class MRTShader;
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
		void FramebuffersCreation_();
		void RecordMRTOffscreen_();
		void RecordFinalFrame_();

		Renderer();
		virtual ~Renderer() {/*do nothing*/}

	private:
		/* swapchain images for drawing and presenting onto the screen */
		VulkanSwapchain* m_swapchain = nullptr;

		/* fbos for gpu to read/write */
		MrtFBO* m_mrtfbo = nullptr;
		std::vector<FinalFBO*> m_fbos;

		/* shader to compute the result of pixels */
		FinalPassShader* m_finalpassshader = nullptr;
		MRTShader* m_mrtshader = nullptr;

		/* recording purpose */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_commandbuffers;
		VkCommandBuffer m_offscreen_cmdbuffer;

		// semaphores for synchronizing read/write images in gpu 
		VkSemaphore m_presentComplete = VK_NULL_HANDLE;
		VkSemaphore m_renderComplete = VK_NULL_HANDLE;
		VkSemaphore m_offscreenComplete = VK_NULL_HANDLE;

		/* a model with mesh to render */
		Model* m_model = nullptr;

		/* a universal UBO */
		UBO* m_ubo = nullptr;

		/* ssbo for instancing data */
		SSBO* m_instance_ssbo = nullptr;

		static std::once_flag m_sflag;
		static Renderer* m_instance;
	};
}

#endif

