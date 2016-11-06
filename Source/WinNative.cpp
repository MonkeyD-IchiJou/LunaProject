#include "WinNative.h"

namespace luna
{
	std::once_flag WinNative::m_sflag{};
	WinNative* WinNative::m_instance = nullptr;

	WinNative::WinNative() : 
		m_win_size_x(1280), 
		m_win_size_y(720),
		m_win_pos_x(90), 
		m_win_pos_y(70),
		m_close(false),
		m_win_surfacesize_x(0),
		m_win_surfacesize_y(0),
		m_win_name("Luna")
	{
	}

	void WinNative::Create()
	{
		// init the os window
		InitOSWindow_();

		// straight away run the window after init
		RunOSWindow_();
	}
}
