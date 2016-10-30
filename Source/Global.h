#ifndef LUNA_GLOBAL_H
#define LUNA_GLOBAL_H

#include <atomic>

namespace luna
{
	namespace global
	{
		extern std::atomic<float> DeltaTime;
	}
}

#endif