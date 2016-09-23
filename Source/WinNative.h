#ifndef WIN_NATIVE_H
#define WIN_NATIVE_H

#include "platform.h"
#include <string>
#include <vector>
#include <mutex>

namespace luna
{
	class Renderer;

	// multiplatform window, singleton
	class WinNative
	{
	public:
		// Update the os window, its input and etc
		void UpdateOSWin();

		inline auto getSurfaceSizeX() const { return m_surface_size_x; }
		inline auto getSurfaceSizeY() const { return m_surface_size_y; }
		inline auto getWinPosX() const { return m_win_pos_x; }
		inline auto getWinPosY() const { return m_win_pos_y; }
		inline auto getName() const { return m_win_name; }
		inline bool isClose() const { return m_close; }
		
		auto setSurfaceSizeX(const uint32_t& val);
		auto setSurfaceSizeY(const uint32_t& val);
		auto setWinPosX(const uint32_t& val);
		auto setWinPosY(const uint32_t& val);
		auto setName(const std::string& name);

#if VK_USE_PLATFORM_WIN32_KHR
		auto getWin32_Instance() const { return m_win32_instance; }
		auto getHWND() const { return m_win32_handle; }
#endif // VK_USE_PLATFORM_WIN32_KHR

		inline auto getSurface() const { return m_surface; }
		inline const auto& getSurfaceCapabilities() const { return m_surface_capabilities; }
		inline const auto& getSurfaceFormats() const { return m_surface_formats; }

		inline auto getSwapChain() const { return m_swapchain; }
		inline const auto& getSwapChainExtent() const { return m_swapchain_extent; }
		inline const auto& getSwapChainImageCount() const { return m_swapchain_image_count; }
		inline const auto& getSwapChainImageViews() const { return m_swapchain_images_views; }

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
		inline void Destroy() { DeInitSwapChain_(); DeInitOSWindowSurface_(); DeInitOSWindow_(); }

	private:
		WinNative();
		~WinNative() {};

		/* platform specific: init the os window */
		void InitOSWindow_();

		/* platform specific: de init and destroy the os window */
		void DeInitOSWindow_();

		/* platform specific: init the os window */
		void UpdateOSWindow_();

		/* platform specific: init the os window surface for vulkan to draw graphics */
		void InitOSWindowSurface_();

		/* Get all the information from the drawing surface */
		void QuerySurfaceInfo_();

		/* Swap chain created with the help of surface info */
		void InitSwapChain_();

		/* get the images created by swap chain */
		void InitSwapChainImages_();

		/* create the image views for the image (handle) */
		void InitSwapChainImageViews_();

		/* deinit the surface */
		void DeInitOSWindowSurface_();

		/* deinit the swap chain, images will be deleted along with it */
		void DeInitSwapChain_();

	private:

		uint32_t m_surface_size_x = 0;
		uint32_t m_surface_size_y = 0;
		uint32_t m_win_pos_x = 0;
		uint32_t m_win_pos_y = 0;
		std::string	m_win_name = " ";
		bool m_close = false;

#if VK_USE_PLATFORM_WIN32_KHR
		HINSTANCE m_win32_instance= NULL;
		HWND m_win32_handle = NULL;
#endif // VK_USE_PLATFORM_WIN32_KHR

		/* window surface and its images */
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkSurfaceCapabilitiesKHR m_surface_capabilities = {};
		VkSurfaceFormatKHR m_surface_formats = {};
		std::vector<VkPresentModeKHR> m_presentmode_list;

		/* window surface swapchain */
		VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
		VkExtent2D m_swapchain_extent = {};
		uint32_t m_swapchain_image_count = 2;

		/* images object in the swap chain */
		std::vector<VkImage> m_swapchain_images;
		std::vector<VkImageView> m_swapchain_images_views;

		const Renderer* renderer_handle = nullptr;

		static std::once_flag m_sflag;
		static WinNative* m_instance;
	};
}

#endif

