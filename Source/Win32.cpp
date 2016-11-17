#include "WinNative.h"
#include "DebugLog.h"
#include "Input.h"
#include "LunaManager.h"

#if VK_USE_PLATFORM_WIN32_KHR

#include <windowsx.h>

namespace luna
{
	LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
	{
		// pointer to the window data
		WinNative * pwin = reinterpret_cast<WinNative*>(
			GetWindowLongPtrW(hwnd, GWLP_USERDATA)
			);

		return pwin->WindowProc(hwnd, msg, wparam, lparam);
	}

	WINDOWPLACEMENT g_wpPrev = { sizeof(g_wpPrev) };
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

			// full screen toggle
			if(wparam == 'F')
			{
				DWORD dwStyle = GetWindowLong(hwnd, GWL_STYLE);
				if (dwStyle & WS_OVERLAPPEDWINDOW) 
				{
					MONITORINFO mi = { sizeof(mi) };
					if (GetWindowPlacement(hwnd, &g_wpPrev) && GetMonitorInfo(MonitorFromWindow(hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) 
					{
						SetWindowLong(hwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
						SetWindowPos(hwnd, HWND_TOP,
							mi.rcMonitor.left, mi.rcMonitor.top,
							mi.rcMonitor.right - mi.rcMonitor.left,
							mi.rcMonitor.bottom - mi.rcMonitor.top,
							SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
					}
				} 
				else 
				{
					SetWindowLong(hwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
					SetWindowPlacement(hwnd, &g_wpPrev);
					SetWindowPos(hwnd, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
						SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
				}
			}
			break;

		case WM_SIZE:
			setWinDrawingSurfaceSize(LOWORD(lparam), HIWORD(lparam));
			break;

		case WM_KEYUP:
			input::Keys[wparam].pressed = false;
		
			break;

		case WM_LBUTTONDOWN:
			input::Mouse.firsttouchposx = LOWORD(lparam);
			input::Mouse.firsttouchposy = HIWORD(lparam);
			input::Mouse.leftclick = true;
			input::Mouse.numTouchPoints = 1;
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

		m_win32_handle = CreateWindowEx(
			0,
			m_win_name.c_str(),				// class name
			m_win_name.c_str(),				// app name
			style,							// window style
			m_win_pos_x, m_win_pos_y,		// x/y coords
			m_win_size_x,					// width
			m_win_size_y,					// height
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

	void WinNative::RunOSWindow_()
	{
		if (m_gamemanager == nullptr)
		{
			// game running in other worker thread
			m_gamemanager = new LunaManager();
			m_gamemanager->GameRun();
		}

		MSG msg{};
		BOOL bRet{};

		// always listening to inputs and events
		while ((bRet = GetMessage(&msg, NULL, 0, 0)) != 0)
		{
			if (bRet == -1)
			{
				// handle the error and possibly exit
				luna::DebugLog::throwEx("windows input error");
			}
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
	}

	void WinNative::setWinDrawingSurfaceSizeX(const uint32_t & val)
	{
		if (m_win_surfacesize_x != val)
		{
			m_win_surfacesize_x = val;
			m_gamemanager->OnWindowSizeChange(m_win_surfacesize_x, m_win_surfacesize_y);
		}
	}

	void WinNative::setWinDrawingSurfaceSizeY(const uint32_t & val)
	{
		if (m_win_surfacesize_y != val)
		{
			m_win_surfacesize_y = val;
			m_gamemanager->OnWindowSizeChange(m_win_surfacesize_x, m_win_surfacesize_y);
		}
	}

	void WinNative::setWinDrawingSurfaceSize(const uint32_t & valx, const uint32_t & valy)
	{
		if (valx > 0 && valy > 0)
		{
			if (m_win_surfacesize_x == 0 || m_win_surfacesize_y == 0)
			{
				m_win_surfacesize_x = valx;
				m_win_surfacesize_y = valy;
			}
			else
			{
				m_win_surfacesize_x = valx;
				m_win_surfacesize_y = valy;

				m_gamemanager->OnWindowSizeChange(valx, valy);
			}
		}
	}

	void WinNative::setWinPosX(const uint32_t & val)
	{
	}

	void WinNative::setWinPosY(const uint32_t & val)
	{
	}

	void WinNative::setName(const std::string & name)
	{
	}

	void WinNative::DeInitOSWindow_()
	{
		// destroy the game manager
		if (m_gamemanager != nullptr)
		{
			delete m_gamemanager;
			m_gamemanager = nullptr;
		}

		if (m_win32_handle != NULL)
		{
			DestroyWindow(m_win32_handle);
			UnregisterClass(m_win_name.c_str(), m_win32_instance);
			m_win32_handle = NULL;
		}
	}
}

#endif //VK_USE_PLATFORM_WIN32_KHR