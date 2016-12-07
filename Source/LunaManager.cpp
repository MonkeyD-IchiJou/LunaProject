#include "LunaManager.h"
#include "DebugLog.h"
#include "Renderer.h"
#include "SceneDefault.h"
#include "WinNative.h"
#include "Global.h"

#define RESOURCEUPDATE0_WORKERID 0
#define RESOURCEUPDATE1_WORKERID 1
#define RESOURCEUPDATE2_WORKERID 2
#define RESOURCEUPDATE3_WORKERID 3
#define RENDER_WORKERID 4
#define GAMELOOPING_WORKERID 5
#define USE_BASICLOOP 1

namespace luna
{
	LunaManager::LunaManager()
	{
		// vulkan first init here
		m_renderer = luna::Renderer::getInstance();

		// init the resources first
		workers[GAMELOOPING_WORKERID].addJob([&]() {
			InitResources_();
		});
	}

	LunaManager::~LunaManager()
	{
		DeInitResources_();
	}

	void LunaManager::GameRun()
	{
		// last worker always looping the game loop
		workers[GAMELOOPING_WORKERID].addJob([&]() {
			GameLoop_();
		});
	}

	void LunaManager::OnWindowSizeChange(const uint32_t & x, const uint32_t & y)
	{
		workers[RENDER_WORKERID].addJob([&]() {
			m_renderer->RecreateSwapChain();
		});
	}

	void LunaManager::GameLoop_()
	{
#ifdef USE_BASICLOOP

		auto win = WinNative::getInstance();

		// framepacket datas
		std::array<FramePacket, 2> framepackets{};
		int updateFrame = 0;
		int renderFrame = 0;

		// first set of worker
		std::array<Worker*, 2> firstset{};
		firstset[0] = &workers[RESOURCEUPDATE0_WORKERID];
		firstset[1] = &workers[RESOURCEUPDATE1_WORKERID];

		// second set of worker
		std::array<Worker*, 2> secondset{};
		secondset[0] = &workers[RESOURCEUPDATE2_WORKERID];
		secondset[1] = &workers[RESOURCEUPDATE3_WORKERID];

		// timer preparation
		using clock = std::chrono::steady_clock;

		std::chrono::nanoseconds lag(0);
		clock::time_point time_start = clock::now();

		// we use a fixed timestep of 1 / (60 fps) = 16 milliseconds
		constexpr std::chrono::nanoseconds timestep(16666666);

		while (!win->isClose())
		{
			// get the prev frame time elapsed
			global::DeltaTime = std::chrono::duration_cast<std::chrono::microseconds>(clock::now() - time_start).count() / 1000000.f;

			// start of this frame
			time_start = clock::now();

			m_scene->Update(framepackets[updateFrame], firstset);
			
			// wait for the previous frame to render finish
			workers[RENDER_WORKERID].wait();

			// setter which framepacket to use 
			renderFrame = updateFrame;
			updateFrame = (updateFrame == 0 ? 1 : 0);

			// Submit this frame!!
			// RENDER_WORKERID worker always queue submit and present it on the screen
			workers[RENDER_WORKERID].addJob([&]() {
				// update datas to the gpu
				m_renderer->RecordAndRender(framepackets[renderFrame], secondset);
			});
		}

		// wait for all the worker except the game looping one
		for (int i = 0; i < GAMELOOPING_WORKERID; ++i)
		{
			workers[i].wait();
		}

#else

		auto win = WinNative::getInstance();

		// framepacket datas
		FramePacket framepacket{};

		// first set of worker
		std::array<Worker*, 2> firstset{};
		firstset[0] = &workers[RESOURCEUPDATE0_WORKERID];
		firstset[1] = &workers[RESOURCEUPDATE1_WORKERID];

		// second set of worker
		std::array<Worker*, 2> secondset{};
		secondset[0] = &workers[RESOURCEUPDATE2_WORKERID];
		secondset[1] = &workers[RESOURCEUPDATE3_WORKERID];

		// timer preparation
		using clock = std::chrono::steady_clock;

		std::chrono::nanoseconds lag(0);
		clock::time_point time_start = clock::now();

		// we use a fixed timestep of 1 / (60 fps) = 16 milliseconds
		constexpr std::chrono::nanoseconds timestep(int(0.016 * 1000000000));
		global::DeltaTime = 0.016f;
		int updateloop = 0;

		// pre update
		m_scene->Update(framepacket, firstset);

		while (!win->isClose())
		{
			// calc how much time have elapsed from prev frame
			auto delta_time = clock::now() - time_start;

			// start of this frame
			time_start = clock::now();

			// add it until more than timestep
			lag += delta_time;

			// update game logic as lag permits
			// Fix Time Stamp!!
			while (lag >= timestep)
			{
				// update my scenes
				m_scene->Update(framepacket, firstset);

				// check how many loops alr
				updateloop++;

				// reduce it until less than my time stamp
				lag -= timestep;
			}

			updateloop = 0;

			workers[RENDER_WORKERID].addJob([&]() {
				// update datas to the gpu
				m_renderer->RecordAndRender(framepacket, secondset);
			});
			workers[RENDER_WORKERID].wait();
		}

		// wait for all the worker except the game looping one
		for (int i = 0; i < GAMELOOPING_WORKERID; ++i)
		{
			workers[i].wait();
		}
#endif

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
		// last worker is special
		workers[GAMELOOPING_WORKERID].wait();
		
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
