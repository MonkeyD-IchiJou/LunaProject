#ifndef WIN_NATIVE_H
#define WIN_NATIVE_H

#include "platform.h"
#include <string>
#include <vector>
#include <mutex>
#include <atomic>

namespace luna
{
	class VulkanRenderer;

	// multiplatform window, singleton
	class WinNative
	{
	public:
		inline uint32_t getWinSizeX() const { return m_win_size_x; }
		inline uint32_t getWinSizeY() const { return m_win_size_y; }
		inline uint32_t getWinPosX() const { return m_win_pos_x; }
		inline uint32_t getWinPosY() const { return m_win_pos_y; }
		inline auto getName() const { return m_win_name; }
		inline bool isClose() const { return m_close; }
		inline auto getSurface() const { return m_surface; }

		auto setWinSizeX(const uint32_t& val);
		auto setWinSizeY(const uint32_t& val);
		auto setWinPosX(const uint32_t& val);
		auto setWinPosY(const uint32_t& val);
		auto setName(const std::string& name);

#if VK_USE_PLATFORM_WIN32_KHR
		auto getWin32_Instance() const { return m_win32_instance; }
		auto getHWND() const { return m_win32_handle; }
		LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
#endif // VK_USE_PLATFORM_WIN32_KHR

#if VK_USE_PLATFORM_ANDROID_KHR
		
		

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
			DeInitWindowSurface_();  DeInitOSWindow_();
		}

	private:
		WinNative();
		~WinNative() {};

		/* platform specific: init the os window */
		void InitOSWindow_();

		/* platform specific: init the os surface */
		void InitOSWindowSurface_();

		/* platform specific: de init and destroy the os window */
		void DeInitOSWindow_();

		/* deinit the window surface */
		void DeInitWindowSurface_();
		
	private:

		std::atomic<uint32_t> m_win_size_x = 0;
		std::atomic<uint32_t> m_win_size_y = 0;
		std::atomic<uint32_t> m_win_pos_x = 0;
		std::atomic<uint32_t> m_win_pos_y = 0;
		std::atomic<bool> m_close = false;

		std::string	m_win_name = " ";
		
#if VK_USE_PLATFORM_WIN32_KHR
		HINSTANCE m_win32_instance= NULL;
		HWND m_win32_handle = NULL;
#endif // VK_USE_PLATFORM_WIN32_KHR

		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkInstance m_vulkanInstance = VK_NULL_HANDLE;

		static std::once_flag m_sflag;
		static WinNative* m_instance;
	};
}

#endif

