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
		using clock = std::chrono::steady_clock;
		std::chrono::nanoseconds lag(0);
		clock::time_point time_start = clock::now();

#if VK_USE_PLATFORM_WIN32_KHR

		auto window = luna::WinNative::getInstance();

		// scene early update first
		m_scene->EarlyUpdate();

		while (!window->isClose())
		{
			// input checking
			MSG msg{};
			if (PeekMessage(&msg, window->getHWND(), 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			else
			{
				// calc how much time have elapsed from prev frame
				auto delta_time = clock::now() - time_start;
				time_start = clock::now(); // start of this frame

				// update game logic
				m_scene->Update();

				// begin to render everything and present it on screen
				m_scene->Render();

				// late update after render 
				m_scene->LateUpdate();

				DebugLog::printL(std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count());
			}
		}
#endif
	}

	void LunaManager::Init_()
	{
		// auto init vulkan when first getinstance
		auto renderer = luna::Renderer::getInstance();

		// auto create the window and its surface when first getinstance
		luna::WinNative::getInstance();

		// create necessary resources, models, ubo, textures, shaders, swapchain, fbos .. all must loaded here and once only 
		renderer->CreateResources();

		// create scenes and prepare entities
		m_scene = new SceneDefault();
	}

	void LunaManager::DeInit_()
	{
		/* Major Clean Up*/
		luna::Renderer::getInstance()->CleanUpResources();

		if (m_scene != nullptr)
		{
			delete m_scene;
			m_scene = nullptr;
		}

		luna::WinNative::getInstance()->Destroy();
		luna::Renderer::getInstance()->Destroy();
	}
}
