#ifndef LUNA_GLOBAL_H
#define LUNA_GLOBAL_H

#include "platform.h"
#include <atomic>

#if VK_USE_PLATFORM_ANDROID_KHR
#include <android_native_app_glue.h>
#endif

namespace luna
{
	namespace global
	{
		extern std::atomic<float> DeltaTime;

#if VK_USE_PLATFORM_ANDROID_KHR
		extern android_app* androidApplication;
#endif
	}
}

#endif