#include "WinNative.h"
#include "DebugLog.h"
#include "Global.h"
#include "Input.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR

namespace luna
{
	static void Callback_AppEvent(android_app* pApplication, int32_t pCommand) 
	{
		WinNative& win = *(WinNative*) pApplication->userData;
		win.AndroidEventProc(pCommand);
	}

	static int32_t handleAppInput(android_app* app, AInputEvent* pEvent)
	{
		int32_t eventType = AInputEvent_getType(pEvent);

		switch (eventType) 
		{
		case AINPUT_EVENT_TYPE_MOTION:

			switch (AInputEvent_getSource(pEvent)) 
			{
			case AINPUT_SOURCE_TOUCHSCREEN:
				
				int action = AKeyEvent_getAction(pEvent) & AMOTION_EVENT_ACTION_MASK;

				switch (action)
				{
				case AMOTION_EVENT_ACTION_DOWN:
					// touch screen input
					input::Mouse.leftclick = true;
					input::Mouse.posx = AMotionEvent_getX(pEvent, 0);
					input::Mouse.posy = AMotionEvent_getY(pEvent, 0);
					input::Mouse.firsttouchposx = AMotionEvent_getX(pEvent, 0);
					input::Mouse.firsttouchposy = AMotionEvent_getY(pEvent, 0);
					
					break;

				case AMOTION_EVENT_ACTION_UP:
					input::Mouse.leftclick = false;
					input::Mouse.lasttouchposx = AMotionEvent_getX(pEvent, 0);
					input::Mouse.lasttouchposy = AMotionEvent_getY(pEvent, 0);
					
					break;

				case AMOTION_EVENT_ACTION_MOVE:
					input::Mouse.posx = AMotionEvent_getX(pEvent, 0);
					input::Mouse.posy = AMotionEvent_getY(pEvent, 0);
					break;
				}

				break;
			}

			return 1;
			break;
		}

		return 0;
	}

	void WinNative::InitOSWindow_()
	{
		DebugLog::printFF("Creating windows");

		m_androidApplication = luna::global::androidApplication;

		m_androidApplication->userData = this;
		m_androidApplication->onAppCmd = Callback_AppEvent;
		m_androidApplication->onInputEvent = handleAppInput;
	}

	void WinNative::InitOSWindowSurface_()
	{
		VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.window = m_androidApplication->window;
		err = vkCreateAndroidSurfaceKHR(instance, &surfaceCreateInfo, NULL, &surface);

		if(err == VK_SUCCESS)
			DebugLog::printFF("create Surface Success");
	}

	void WinNative::AndroidEventProc(int32_t pCommand)
	{
		switch (pCommand) 
		{
		case APP_CMD_SAVE_STATE:
			DebugLog::printFF("CMD SAVE APP !!");
			break;

		case APP_CMD_INIT_WINDOW:
			DebugLog::printFF("Window Init App");
			break;

		case APP_CMD_GAINED_FOCUS:
			DebugLog::printFF("Gained Focus App");
			m_focus = true;
			break;

		case APP_CMD_LOST_FOCUS:
			DebugLog::printFF("Lost Focus App");
			m_focus = false;
			break;
		case APP_CMD_TERM_WINDOW:
			DebugLog::printFF("APP_CMD_TERM_WINDOW");
			break;
		}
	}

	void WinNative::setWinSizeX(const uint32_t & val)
	{
	}

	void WinNative::setWinSizeY(const uint32_t & val)
	{
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
	}
}

#endif // VK_USE_PLATFORM_ANDROID_KHR

