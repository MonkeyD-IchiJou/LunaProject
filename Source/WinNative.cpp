#include "WinNative.h"
#include "Renderer.h"
#include "DebugLog.h"

namespace luna
{
	std::once_flag WinNative::m_sflag{};
	WinNative* WinNative::m_instance = nullptr;

	WinNative::WinNative() : 
		m_win_size_x(1280), 
		m_win_size_y(720), 
		m_win_pos_x(90), 
		m_win_pos_y(70),
		m_win_name("Luna")
	{			
		/* get the reference of the vulkan instance */
		m_vulkanInstance = Renderer::getInstance()->GetVulkanInstance();

		// init the os window
		InitOSWindow_();	 

		// surface inits 
		InitOSWindowSurface_();
	}

	void WinNative::UpdateOSWin()
	{
		// update the os window
		UpdateOSWindow_();
	}

	void WinNative::DeInitWindowSurface_()
	{
		if (m_surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(m_vulkanInstance, m_surface, nullptr);
			m_surface = VK_NULL_HANDLE;
		}
	}
}
