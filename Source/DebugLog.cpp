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

	void DebugLog::printFF(const char* pMessage, ...)
	{
#if VK_USE_PLATFORM_ANDROID_KHR
			va_list varArgs;
			va_start(varArgs, pMessage);
			__android_log_vprint(ANDROID_LOG_INFO, "LunaInfo", pMessage, varArgs);
			va_end(varArgs);
#endif
	}
}
