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
		clock::time_point time_start = clock::now();

		auto renderer = luna::Renderer::getInstance();

		FramePacket framepacket{};

		// scene early update first
		m_scene->EarlyUpdate(framepacket);

		// update to buffer objects
		renderer->MapGeometryDatas(framepacket.instancedatas);
		renderer->MapFontInstDatas(framepacket.fontinstancedatas);
		renderer->MapMainCamDatas(framepacket.maincamdata);

		// rmb to re-record the command buffer again if m_renderinfos is different
		renderer->RecordTransferData_Secondary();
		renderer->RecordGeometryPass_Secondary(framepacket.renderinfos);
		renderer->RecordUIPass_Secondary(static_cast<uint32_t>(framepacket.fontinstancedatas.size()));

#if VK_USE_PLATFORM_WIN32_KHR

		auto window = luna::WinNative::getInstance();

		while (!window->isClose())
		{
			// calc how much time have elapsed from prev frame
			auto delta_time = clock::now() - time_start;

			// start of this frame
			time_start = clock::now(); 

			// update game logic
			m_scene->Update(framepacket);

			// begin to render everything and present it on screen
			renderer->Render();

			DebugLog::printL(delta_time.count());
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
				DebugLog::throwEx("windows input error");
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
