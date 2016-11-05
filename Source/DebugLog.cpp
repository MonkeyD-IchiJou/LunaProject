#include "DebugLog.h"
#include <stdlib.h>
#include <stdarg.h>

#if VK_USE_PLATFORM_ANDROID_KHR
#include <android/log.h>
#endif

namespace luna
{

	DebugLog::DebugLog()
	{
	}

	DebugLog::~DebugLog()
	{
	}

	void DebugLog::printF(const char* pMessage, ...)
	{
		va_list varArgs;
		va_start(varArgs, pMessage);

#if VK_USE_PLATFORM_ANDROID_KHR
		__android_log_vprint(ANDROID_LOG_INFO, "LunaInfo", pMessage, varArgs);
#elif VK_USE_PLATFORM_WIN32_KHR
#if _DEBUG
		printf(pMessage, varArgs);
#endif
#endif
		va_end(varArgs);
	}
}
