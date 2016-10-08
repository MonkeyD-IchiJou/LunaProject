#ifndef FONT_H
#define FONT_H

#include <array>
#include <string>
#include <glm\glm.hpp>

namespace luna
{
	struct bmchar 
	{
		uint32_t x, y;
		uint32_t width;
		uint32_t height;
		int32_t xoffset;
		int32_t yoffset;
		int32_t xadvance;
		uint32_t page;
	};

	struct vulkanchar
	{
		glm::vec2 uv[4];
		glm::vec2 size;
		glm::vec2 halfsize;
		float xoffset;
		float yoffset;
		float xadvance; // for the next char to start position with 
	};

	class Font
	{
	public:
		Font();
		~Font();

		// load the fonts file and fill all the datas
		void LoadFonts(const std::string& fileName, const float& texwidth, const float& texheight);

		// all the vulkan chars' datas store here
		std::array<vulkanchar, 255> vulkanChars{};

	private:
		static int32_t nextValuePair(std::stringstream *stream);
	};
}

#endif

