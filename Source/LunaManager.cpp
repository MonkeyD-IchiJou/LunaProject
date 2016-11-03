#include "LunaManager.h"
#include "DebugLog.h"
#include "Renderer.h"
#include "WinNative.h"
#include "SceneDefault.h"
#include "Worker.h"
#include "Global.h"
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
		if(GameThread.joinable())
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
		clock::time_point time_start = clock::now();

		auto renderer = luna::Renderer::getInstance();

		// framepacket datas
		FramePacket framepacket{};

		// i have 3 workers waiting to do jobs
		std::array<Worker, 3> workers{};

		auto window = luna::WinNative::getInstance();

#if VK_USE_PLATFORM_ANDROID_KHR 
		while(true)

#elif VK_USE_PLATFORM_WIN32_KHR
		while (!window->isClose())
#endif

		{
			// get the prev frame time elapsed
			global::DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - time_start).count() / 1000000.f;

			// start of this frame
			time_start = clock::now(); 

			// update game logic
			m_scene->Update(framepacket, workers);

			
			// update the gpu
			renderer->RecordBuffers(framepacket, workers);
			
			DebugLog::printFF("Begin Rendering");

			// queue submit and present it on the screen
			renderer->Render();
		}
	}

	void LunaManager::InputRun_()
	{
		auto win = luna::WinNative::getInstance();

#if VK_USE_PLATFORM_WIN32_KHR
		MSG msg;
		BOOL bRet;

		while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
		{
			if (bRet == -1)
			{
				// handle the error and possibly exit
				DebugLog::throwEx("windows input error");
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
#else

		int32_t result = 0;
		int32_t events = 0;
		android_poll_source* source = nullptr;
		android_app* androidapp = luna::global::androidApplication;

		while (true)
		{
			// event processing loop
			while ((result = ALooper_pollAll(0, NULL, &events, (void**)&source)) >= 0)
			{
				// an even has to be processed
				if (source != nullptr)
				{
					//luna::DebugLog::printFF("Processing an event");
					source->process(androidapp, source);
				}

				// application is getting destroyed
				if (androidapp->destroyRequested != 0)
				{
					DebugLog::printFF("Exiting event loop");
					return;
				}
			}
		}

		luna::DebugLog::printFF("Destroy liao");

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
		DebugLog::printFF("Resources create finish");

		// create scenes and prepare entities
		m_scene = new SceneDefault();
		DebugLog::printFF("Scenes create finish");
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
