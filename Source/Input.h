#ifndef INPUT_LUNA_H
#define INPUT_LUNA_H

#include "platform.h"
#include <atomic>
#include <array>

namespace luna
{
	namespace input
	{
		struct keyinfo
		{
			std::atomic<bool> pressed = false;
		};

		// 256 different keys registered
		extern std::array<keyinfo, 256> Keys;

		struct mouseinfo
		{
			// mouse position
			std::atomic<int> posx{};
			std::atomic<int> posy{};

			// first touch position
			std::atomic<int> firsttouchposx{};
			std::atomic<int> firsttouchposy{};

			// last touch position
			std::atomic<int> lasttouchposx{};
			std::atomic<int> lasttouchposy{};

			std::atomic<bool> leftclick = false;
			std::atomic<bool> leftdbclick = false;

			std::atomic<bool> rightclick = false;
			std::atomic<bool> rightdbclick = false;

			std::atomic<int> scrolldelta = 0;
		};

		extern mouseinfo Mouse;
	}

	//case '\b': // backspace
	//case '\t': // tab
	//case '\n':
	//case '\r': // enter
}

#endif