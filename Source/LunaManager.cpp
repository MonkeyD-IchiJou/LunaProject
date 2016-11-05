#include "LunaManager.h"
#include "DebugLog.h"
#include "Renderer.h"
#include "SceneDefault.h"
#include "WinNative.h"
#include "Global.h"
#include <chrono>

namespace luna
{
	LunaManager::LunaManager()
	{
		// vulkan first init here
		m_renderer = luna::Renderer::getInstance();

		// init the resources first
		workers[3].addJob([&]() {
#if _DEBUG
			try
			{
				InitResources_();
			}
			catch (const std::runtime_error& e)
			{
				DebugLog::printF("[Worker 3 Thread Catch]->");
				DebugLog::printF(e.what());
				system("pause");
			}
#else
			InitResources_();
#endif
		});
	}

	LunaManager::~LunaManager()
	{
		DeInitResources_();
	}

	void LunaManager::GameRun()
	{
		// last worker always looping the game loop
		workers[3].addJob([&]() {
#if _DEBUG
			try
			{
				GameLoop_();
			}
			catch (const std::runtime_error& e)
			{
				DebugLog::printF("[Worker 3 Thread Catch]->");
				DebugLog::printF(e.what());
				system("pause");
			}
#else
			GameLoop_();
#endif
		});
	}

	void LunaManager::GameLoop_()
	{
		using clock = std::chrono::steady_clock;
		clock::time_point time_start = clock::now();

		auto win = WinNative::getInstance();

		// framepacket datas
		FramePacket framepacket{};

		while (!win->isClose())
		{
			// get the prev frame time elapsed
			global::DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - time_start).count() / 1000000.f;

			// start of this frame
			time_start = clock::now();

			// update game logic
			m_scene->Update(framepacket, workers);

			// update the gpu
			m_renderer->RecordBuffers(framepacket, workers);

			// queue submit and present it on the screen
			m_renderer->Render();
		}
	}

	void LunaManager::InitResources_()
	{
		// create necessary resources, models, ubo, textures, shaders, surface, swapchain, fbos .. all must loaded here and once only 
		m_renderer->CreateResources();

		// create scenes and prepare entities
		m_scene = new SceneDefault();
	}

	void LunaManager::DeInitResources_()
	{
		// wait for all the workers to finish first
		for (int i = 0; i < 4; ++i)
		{
			workers[i].wait();
		}

		if (m_scene != nullptr)
		{
			delete m_scene;
			m_scene = nullptr;
		}

		/* Major Clean Up*/
		m_renderer->Destroy();
		m_renderer = nullptr;
	}
}
