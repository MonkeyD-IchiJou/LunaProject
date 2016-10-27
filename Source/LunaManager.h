#ifndef LUNA_MANAGER_H
#define LUNA_MANAGER_H

#include <mutex>

namespace luna
{
	class Renderer;
	class WinNative;
	class Scene;

	class LunaManager
	{
	public:
		void Run();

	public:
		/* Singleton class implementation */
		static inline LunaManager* getInstance(void)
		{
			// only called once
			std::call_once(m_sflag, [&]() {
				m_instance = new LunaManager();
			});

			return m_instance;
		}

		/* check whether exist or not */
		static inline bool exists(void)
		{
			return m_instance != nullptr;
		}

		/* Warning Once destroyed, forever destroy */
		inline void Destroy() { DeInit_(); }

	private:
		void Init_();
		void DeInit_();

		void GameRun_();
		void GameLoop_();
		void InputRun_();

		LunaManager();
		~LunaManager() {/* do nothing */}

	private:
		/* all the scenes are here */
		Scene* m_scene = nullptr;

		static std::once_flag m_sflag;
		static LunaManager* m_instance;
	};
}

#endif // !LUNA_MANAGER_H


