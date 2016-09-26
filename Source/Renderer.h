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
		Renderer();
		virtual ~Renderer();

		/* create swapchain, shaders, fbo, renderpass */
		void CreateResources() override;

		/* delete swap chain, fbo, shaders and renderpass*/
		void CleanUpResources() override;


		void RenderSetup();
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

	private:
		VulkanSwapchain* m_swapchain = nullptr;
		std::vector<BaseFBO*> m_fbos;
		BasicShader* m_shader = nullptr;
		VkRenderPass m_renderpass = VK_NULL_HANDLE;

		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> m_commandbuffers;

		VkSemaphore m_imageAvailableSemaphore = VK_NULL_HANDLE;
		VkSemaphore m_renderFinishSemaphore = VK_NULL_HANDLE;

		/* a quad mesh to render */
		Model* m_quad = nullptr;
		BasicUBO* m_ubo = nullptr;

		static std::once_flag m_sflag;
		static Renderer* m_instance;
	};
}

#endif

