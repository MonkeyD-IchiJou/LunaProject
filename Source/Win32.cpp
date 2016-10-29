#include "WinNative.h"
#include "DebugLog.h"
#include "Renderer.h"
#include "Input.h"

#if VK_USE_PLATFORM_WIN32_KHR

#include <windowsx.h>

namespace luna
{
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		// TODO: mouse scolling input

		// pointer to the window data
		WinNative * pwin = reinterpret_cast<WinNative*>(
			GetWindowLongPtrW(hwnd, GWLP_USERDATA)
			);

		return pwin->WindowProc(hwnd, msg, wparam, lparam);
	}

	LRESULT WinNative::WindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		int i = 0;

		switch (msg)
		{
		case WM_DESTROY:
			close();
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
			// press is confirm true
			input::Keys[wparam].pressed = true;

			//int hold = (lparam & (1 << 30)) >> 30;
		
			break;

		case WM_KEYUP:
			input::Keys[wparam].pressed = false;
		
			break;

		case WM_LBUTTONDOWN:
			input::Mouse.firsttouchposx = LOWORD(lparam);
			input::Mouse.firsttouchposy = HIWORD(lparam);
			input::Mouse.leftclick = true;
			break;

		case WM_LBUTTONDBLCLK:
			input::Mouse.firsttouchposx = LOWORD(lparam);
			input::Mouse.firsttouchposy = HIWORD(lparam);
			input::Mouse.leftdbclick = true;
			input::Mouse.leftclick = true;
			break;

		case WM_RBUTTONDOWN:
			input::Mouse.firsttouchposx = LOWORD(lparam);
			input::Mouse.firsttouchposy = HIWORD(lparam);
			input::Mouse.rightclick = true;
			break;

		case WM_RBUTTONDBLCLK:
			input::Mouse.firsttouchposx = LOWORD(lparam);
			input::Mouse.firsttouchposy = HIWORD(lparam);
			input::Mouse.rightdbclick = true;
			input::Mouse.rightclick = true;
			break;

		case WM_LBUTTONUP:
			input::Mouse.lasttouchposx = LOWORD(lparam);
			input::Mouse.lasttouchposy = HIWORD(lparam);
			input::Mouse.leftclick = false;
			input::Mouse.leftdbclick = false;
			break;

		case WM_RBUTTONUP:
			input::Mouse.lasttouchposx = LOWORD(lparam);
			input::Mouse.lasttouchposy = HIWORD(lparam);
			input::Mouse.rightclick = false;
			input::Mouse.rightdbclick = false;
			break;

		case WM_MOUSEMOVE:
			input::Mouse.posx = LOWORD(lparam);
			input::Mouse.posy = HIWORD(lparam);
			break;

		case WM_MOUSEWHEEL:
			input::Mouse.scrolldelta = GET_WHEEL_DELTA_WPARAM(wparam);
			break;

		default:
			return DefWindowProc(hwnd, msg, wparam, lparam);
			break;
		}

		return 0;
	}

	void WinNative::InitOSWindow_()
	{
		if (m_win_size_x <= 0 || m_win_size_y <= 0)
		{
			DebugLog::throwEx("window size is undefined");
		}

		WNDCLASSEX win_class{};
		m_win32_instance = GetModuleHandle(nullptr);

		// Initialize the window class structure:
		win_class.cbSize = sizeof(WNDCLASSEX);
		win_class.style	= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		win_class.lpfnWndProc = WndProc;
		win_class.cbClsExtra = 0;
		win_class.cbWndExtra = 0;
		win_class.hInstance	= m_win32_instance; // hInstance
		win_class.hIcon	= LoadIcon(NULL, IDI_APPLICATION);
		win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		win_class.hbrBackground	= (HBRUSH)GetStockObject(NULL_BRUSH); // dun clean any thing pls
		win_class.lpszMenuName = NULL;
		win_class.lpszClassName	= m_win_name.c_str();
		win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

		// Register window class:
		if (!RegisterClassEx(&win_class)) 
		{
			DebugLog::throwEx("Cannot create a window in which to draw!\n");
		}

		DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		DWORD style = WS_OVERLAPPEDWINDOW;

		// Create window with the registered class:
		RECT wr = { 0, 0, LONG(m_win_size_x), LONG(m_win_size_y) };
		AdjustWindowRectEx(&wr, style, FALSE, ex_style);

		m_win32_handle = CreateWindowEx(
			0,
			m_win_name.c_str(),				// class name
			m_win_name.c_str(),				// app name
			style,							// window style
			m_win_pos_x, m_win_pos_y,		// x/y coords
			wr.right - wr.left,				// width
			wr.bottom - wr.top,				// height
			NULL,							// handle to parent
			NULL,							// handle to menu
			m_win32_instance,				// hInstance
			NULL							// no extra parameters
		);							

		if (!m_win32_handle) 
		{
			DebugLog::throwEx("Cannot create a window in which to draw!\n");
		}

		SetWindowLongPtr(m_win32_handle, GWLP_USERDATA, (LONG_PTR)this);

		ShowWindow(m_win32_handle, SW_SHOW);
		SetForegroundWindow(m_win32_handle);
		SetFocus(m_win32_handle);
	}

	void WinNative::InitOSWindowSurface_()
	{
		VkWin32SurfaceCreateInfoKHR createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		createInfo.hinstance = m_win32_instance;
		createInfo.hwnd	= m_win32_handle;

		vkCreateWin32SurfaceKHR(m_vulkanInstance, &createInfo, nullptr, &m_surface);
	}

	auto WinNative::setWinSizeX(const uint32_t & val)
	{
	}

	auto WinNative::setWinSizeY(const uint32_t & val)
	{
	}

	auto WinNative::setWinPosX(const uint32_t & val)
	{
	}

	auto WinNative::setWinPosY(const uint32_t & val)
	{
	}

	auto WinNative::setName(const std::string & name)
	{
	}

	void WinNative::DeInitOSWindow_()
	{
		if (m_win32_handle != NULL)
		{
			DestroyWindow(m_win32_handle);
			UnregisterClass(m_win_name.c_str(), m_win32_instance);
			m_win32_handle = NULL;
		}
	}
}

#endif //VK_USE_PLATFORM_WIN32_KHR