#ifndef WIN_NATIVE_H
#define WIN_NATIVE_H

#include "platform.h"
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace luna
{
	class LunaManager;

	// multiplatform window, singleton
	class WinNative
	{
	public:
		void Create();

		inline uint32_t getWinSurfaceSizeX() const { return m_win_surfacesize_x; }
		inline uint32_t getWinSurfaceSizeY() const { return m_win_surfacesize_y; }
		inline uint32_t getWinPosX() const { return m_win_pos_x; }
		inline uint32_t getWinPosY() const { return m_win_pos_y; }
		inline std::string getName() const { return m_win_name; }
		inline bool isClose() const { return m_close; }

		void setWinDrawingSurfaceSizeX(const uint32_t& val);
		void setWinDrawingSurfaceSizeY(const uint32_t& val);
		void setWinDrawingSurfaceSize(const uint32_t& valx, const uint32_t& valy);
		void setWinPosX(const uint32_t& val);
		void setWinPosY(const uint32_t& val);
		void setName(const std::string& name);

#if VK_USE_PLATFORM_WIN32_KHR
		HINSTANCE getWin32_Instance() const { return m_win32_instance; }
		HWND getHWND() const { return m_win32_handle; }
		LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#endif // VK_USE_PLATFORM_WIN32_KHR

#if VK_USE_PLATFORM_ANDROID_KHR
		void AndroidEventProc(int32_t pCommand);
		bool getFocus() const { return m_focus; }
#endif // VK_USE_PLATFORM_ANDROID_KHR

		// close the window 
		inline void close() { m_close = true; }

	public:
		/* Singleton class implementation */
		static inline WinNative* getInstance(void)
		{
			// only called once
			std::call_once(m_sflag, [&]() {
				m_instance = new WinNative();
			});

			return m_instance;
		}

		/* check whether exist or not */
		static inline bool exists(void)
		{
			return m_instance != nullptr;
		}

		/* Warning Once destroyed, forever destroy */
		inline void Destroy() { 
			DeInitOSWindow_();
		}

	private:
		WinNative();
		~WinNative() {};

		/* platform specific: init the os window */
		void InitOSWindow_();

		/* platform specific: windows running events */
		void RunOSWindow_();

		/* platform specific: de init and destroy the os window */
		void DeInitOSWindow_();
		
	private:

		std::atomic<uint32_t> m_win_size_x;
		std::atomic<uint32_t> m_win_size_y;
		std::atomic<uint32_t> m_win_surfacesize_x;
		std::atomic<uint32_t> m_win_surfacesize_y;
		std::atomic<uint32_t> m_win_pos_x;
		std::atomic<uint32_t> m_win_pos_y;
		std::atomic<bool> m_close;

		std::string	m_win_name = " ";
		
#if VK_USE_PLATFORM_WIN32_KHR
		HINSTANCE m_win32_instance= NULL;
		HWND m_win32_handle = NULL;
#endif // VK_USE_PLATFORM_WIN32_KHR

#if VK_USE_PLATFORM_ANDROID_KHR
		android_app* m_androidApplication = nullptr;
		bool m_focus = true;
#endif // VK_USE_PLATFORM_ANDROID_KHR

		// game manager objects
		LunaManager* m_gamemanager = nullptr;

		static std::once_flag m_sflag;
		static WinNative* m_instance;
	};
}

#endif

