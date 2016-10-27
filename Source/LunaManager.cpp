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
		// run the game loop at other thread
		std::thread GameThread([this] { GameRun_(); });

		// run the input at main thread
		InputRun_();

		// join back the game loop thread
		GameThread.join();
	}

	void LunaManager::GameRun_()
	{
#if _DEBUG 
		try
		{
			GameLoop_();
		}
		catch (const std::runtime_error& e) 
		{
			luna::DebugLog::print("[Game loop Thread Catch]->");
			luna::DebugLog::printL(e.what());
			MessageBox(NULL, e.what(), "[Game loop Thread Catch]->", 0);
			system("pause");
		}

#else //BUILD_ENABLE_DEBUG
		GameLoop_();
#endif
	}

	void LunaManager::GameLoop_()
	{
		using clock = std::chrono::steady_clock;
		std::chrono::nanoseconds lag(0);
		clock::time_point time_start = clock::now();

#if VK_USE_PLATFORM_WIN32_KHR

		// scene early update first
		m_scene->EarlyUpdate();

		auto window = luna::WinNative::getInstance();

		while (!window->isClose())
		{
			// calc how much time have elapsed from prev frame
			auto delta_time = clock::now() - time_start;

			// start of this frame
			time_start = clock::now(); 

			// update game logic
			m_scene->Update();

			// begin to render everything and present it on screen
			m_scene->Render();

			// late update after render 
			m_scene->LateUpdate();

			DebugLog::printL(std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count());
		}
#endif
	}

	void LunaManager::InputRun_()
	{
		auto window = luna::WinNative::getInstance();

#if VK_USE_PLATFORM_WIN32_KHR
		MSG msg;
		BOOL bRet;

		while( (bRet = GetMessage( &msg, NULL, 0, 0 )) != 0)
		{ 
			if (bRet == -1)
			{
				// handle the error and possibly exit
				DebugLog::printL("windows input error");
			}
			else
			{
				TranslateMessage(&msg); 
				DispatchMessage(&msg); 
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
