#include "LunaManager.h"
#include "DebugLog.h"
#include "Renderer.h"
#include "WinNative.h"
#include "SceneDefault.h"

#include <chrono>

namespace luna
{
	std::once_flag LunaManager::m_sflag{};
	LunaManager* LunaManager::m_instance = nullptr;

	LunaManager::LunaManager()
	{
		Init_();
	}

	void LunaManager::Run()
	{
		// command buffer record how it is going to render
		m_renderer->Record();

		using clock = std::chrono::steady_clock;
		std::chrono::nanoseconds lag(0);
		clock::time_point time_start = clock::now();

#if VK_USE_PLATFORM_WIN32_KHR
		while (!m_window->isClose())
		{
			// calc how much time have elapsed from prev frame
			auto delta_time = clock::now() - time_start;
			time_start = clock::now(); // start of this frame

			// input checking
			MSG msg{};
			if (PeekMessage(&msg, m_window->getHWND(), 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				m_scene->Update();

				// update game logic
				m_renderer->Update();
				
				// begin to render everything and present it on screen
				m_renderer->Render();
			}

			DebugLog::printL(std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count());
		}
#endif
	}

	void LunaManager::Init_()
	{
		// auto init vulkan when first getinstance
		m_renderer = luna::Renderer::getInstance();

		// auto create the window and its surface when first getinstance
		m_window = luna::WinNative::getInstance();

		// create necessary resources, models, ubo, textures, shaders, swapchain, fbos .. all must loaded here and once only 
		m_renderer->CreateResources();

		m_scene = new SceneDefault();
	}

	void LunaManager::DeInit_()
	{
		/* Major Clean Up*/
		m_renderer->CleanUpResources();

		if (m_scene != nullptr)
		{
			delete m_scene;
			m_scene = nullptr;
		}

		m_window->Destroy();
		m_window = nullptr;

		m_renderer->Destroy();
		m_renderer = nullptr;
	}
}
