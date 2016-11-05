#include "WinNative.h"
#include "DebugLog.h"
#include "Global.h"
#include "Input.h"
#include "LunaManager.h"

#ifdef VK_USE_PLATFORM_ANDROID_KHR

namespace luna
{
	static void Callback_AppEvent(android_app* pApplication, int32_t pCommand) 
	{
		WinNative& win = *(WinNative*) pApplication->userData;
		win.AndroidEventProc(pCommand);
	}

	void WinNative::AndroidEventProc(int32_t pCommand)
	{
		switch (pCommand) 
		{
			case APP_CMD_SAVE_STATE:
				DebugLog::printF("CMD SAVE APP !!");
				break;

			case APP_CMD_INIT_WINDOW:
				DebugLog::printF("Window Init App");

				if (m_gamemanager == nullptr)
				{
					// game running in other worker thread
					m_gamemanager = new LunaManager();
					m_gamemanager->GameRun();
				}

				break;

			case APP_CMD_GAINED_FOCUS:
				DebugLog::printF("Gained Focus App");
				m_focus = true;
				break;

			case APP_CMD_LOST_FOCUS:
				DebugLog::printF("Lost Focus App");
				m_focus = false;
				break;

			case APP_CMD_TERM_WINDOW:
				DebugLog::printF("APP_CMD_TERM_WINDOW");
				break;

			default:
				break;
		}
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
					{
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

							default:
								break;
						}
					}
						break;

					default:
						break;
				}

				return 1;

			default:
				return 0;
		}
	}

	void WinNative::InitOSWindow_()
	{
		DebugLog::printF("Creating windows");

		m_androidApplication = luna::global::androidApplication;

		m_androidApplication->userData = this;
		m_androidApplication->onAppCmd = Callback_AppEvent;
		m_androidApplication->onInputEvent = handleAppInput;
	}

	void WinNative::RunOSWindow_()
	{
		int32_t events = 0;
		android_poll_source* source = nullptr;
		android_app* androidapp = luna::global::androidApplication;

		// event processing loop // always checking for events
		while (ALooper_pollAll(-1, NULL, &events, (void**)&source) >= 0)
		{
			// an even has to be processed
			if (source != nullptr)
			{
				//luna::DebugLog::printFF("Processing an event");
				source->process(androidapp, source);
			}

			// application is getting destroyed
			if (androidapp->destroyRequested != 0)
			{
				DebugLog::printF("Exiting event loop");
				return;
			}
		}

	}

	void WinNative::setWinSizeX(const uint32_t & val)
	{
		this->m_win_size_x = val;
	}

	void WinNative::setWinSizeY(const uint32_t & val)
	{
		this->m_win_size_y = val;
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
	}
}

#endif // VK_USE_PLATFORM_ANDROID_KHR

