#include "Global.h"

namespace luna
{
	namespace global
	{
		std::atomic<float> DeltaTime = 0.f;
#if VK_USE_PLATFORM_ANDROID_KHR
		android_app* androidApplication = nullptr;
#endif
	}
}
