#include "Font.h"
#include "DebugLog.h"

#include <sstream>
#include <stdio.h>
#include <stdlib.h>

#include "IOManager.h"

namespace luna
{
	Font::Font()
	{
	}

	Font::~Font()
	{
	}

	void Font::LoadFonts(const std::string& fileName, const float& texwidth, const float& texheight)
	{
#if defined(__ANDROID__)

		// Font description file is stored inside the apk
		// So we need to load it using the asset manager
		std::vector<char> buffer;
		IO::LoadFile(path, buffer);

		std::stringbuf sbuf(buffer.data());
		std::istream istream(&sbuf);

#else
		std::filebuf fileBuffer;
		fileBuffer.open(fileName, std::ios::in);
		std::istream istream(&fileBuffer);

#endif

		if (!istream.good())
		{
			DebugLog::throwEx("font file not found");
		}

		uint32_t padding = 20; // normally is 20 // hardcode value here

		while (!istream.eof())
		{
			std::string line;
			std::stringstream lineStream;
			std::getline(istream, line);
			lineStream << line;

			std::string info;
			lineStream >> info;

			if (info == "char")
			{
				std::string pair;

				// char id
				uint32_t charid = nextValuePair(&lineStream);
				
				// Char properties
				bmchar fontchar{};
				fontchar.x = nextValuePair(&lineStream);
				fontchar.y = nextValuePair(&lineStream);
				fontchar.width = nextValuePair(&lineStream);
				fontchar.height = nextValuePair(&lineStream);
				fontchar.xoffset = nextValuePair(&lineStream);
				fontchar.yoffset = nextValuePair(&lineStream);
				fontchar.xadvance = nextValuePair(&lineStream) - padding;
				fontchar.page = nextValuePair(&lineStream);

				if (fontchar.width == 0)
				{
					fontchar.xadvance += padding/2;
				}

				// vulkan char 
				// in uv space
				float x = fontchar.x / texwidth;
				float y = fontchar.y / texheight;
				float width = fontchar.width / texwidth;
				float height = fontchar.height / texheight;
				float xoffset = fontchar.xoffset / texwidth;
				float yoffset = fontchar.yoffset / texheight;
				float xadvance = fontchar.xadvance / texwidth;
				
				// settle the uv coordinates
				vulkanchar& vc = vulkanChars[charid];
				vc.uv[0] = glm::vec2(x, y);
				vc.uv[1] = glm::vec2(x, y + height);
				vc.uv[2] = glm::vec2(x + width, y + height);
				vc.uv[3] = glm::vec2(x + width, y);

				vc.size = glm::vec2(width, height);
				vc.halfsize = vc.size / 2.f;
				vc.xadvance = xadvance;
				vc.xoffset = xoffset;
				vc.yoffset = -yoffset; // upside down
			}
		}
	}

	int32_t Font::nextValuePair(std::stringstream * stream)
	{
		std::string pair;
		*stream >> pair;
		uint32_t spos = static_cast<uint32_t>(pair.find("="));
		std::string value = pair.substr(spos + 1);
		int32_t val = std::stoi(value);
		return val;
	}
}
