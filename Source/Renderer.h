#ifndef RENDERER_H
#define RENDERER_H

#include "VulkanRenderer.h"
#include <mutex>

namespace luna
{
	class VulkanSwapchain;
	class BaseFBO;
	class BasicShader;
	class Model;
	class BasicUBO;

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
		/* render pass to tell the fbo how to use the image views for presenting and rendering */
		void InitFinalRenderPass_();

		Renderer();
		virtual ~Renderer() {/*do nothing*/}

	private:
		/* swapchain images for drawing and presenting onto the screen */
		VulkanSwapchain* m_swapchain = nullptr;

		/* fbos for gpu to read/write */
		std::vector<BaseFBO*> m_fbos;

		/* final shader to compute the result of pixels */
		BasicShader* m_shader = nullptr;

		/* description of how to render with the images */
		VkRenderPass m_renderpass = VK_NULL_HANDLE;

		/* recording purpose */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_commandbuffers;

		// semaphores for synchronizing read/write images in gpu 
		VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore m_renderFinishSemaphore = VK_NULL_HANDLE;

		/* a quad mesh to render */
		Model* m_quad = nullptr;

		/* a universal UBO */
		BasicUBO* m_ubo = nullptr;

		static std::once_flag m_sflag;
		static Renderer* m_instance;
	};
}

#endif

