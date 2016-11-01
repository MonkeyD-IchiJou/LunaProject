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
			std::atomic<bool> pressed;

			keyinfo()
			{
				pressed = false;
			}
		};

		// 256 different keys registered
		extern std::array<keyinfo, 256> Keys;

		struct mouseinfo
		{
			// mouse position
			std::atomic<int> posx;
			std::atomic<int> posy;

			// first touch position
			std::atomic<int> firsttouchposx;
			std::atomic<int> firsttouchposy;

			// last touch position
			std::atomic<int> lasttouchposx;
			std::atomic<int> lasttouchposy;

			std::atomic<bool> leftclick;
			std::atomic<bool> leftdbclick;

			std::atomic<bool> rightclick;
			std::atomic<bool> rightdbclick;

			std::atomic<int> scrolldelta;

			mouseinfo()
			{
				posx = 0;
				posy = 0;

				firsttouchposx = 0;
				firsttouchposy = 0;

				lasttouchposx = 0;
				lasttouchposy = 0;

				leftclick = false;
				leftdbclick = false;

				rightclick = false;
				rightdbclick = false;

				scrolldelta = 0;
			}
		};

		extern mouseinfo Mouse;
	}

	//case '\b': // backspace
	//case '\t': // tab
	//case '\n':
	//case '\r': // enter
}

#endif