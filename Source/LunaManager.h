#ifndef LUNA_MANAGER_H
#define LUNA_MANAGER_H

#include "Worker.h"
#include <array>

namespace luna
{
	class Scene;
	class Renderer;

	class LunaManager
	{
	public:
		LunaManager();
		~LunaManager();

		// game run in other thread
		void GameRun();

		// when window size changes
		void OnWindowSizeChange(const uint32_t& x, const uint32_t& y);

	private:
		void InitResources_();
		void DeInitResources_();

		void GameLoop_();
		
	private:
		/* all the scenes are here */
		Scene* m_scene = nullptr;

		/* renderer handle */
		Renderer* m_renderer = nullptr;

		// i have 4 workers waiting to do jobs
		std::array<Worker, 6> workers{};
	};
}

#endif // !LUNA_MANAGER_H


