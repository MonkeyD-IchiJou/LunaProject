#ifndef FONT_COMPONENT_H
#define FONT_COMPONENT_H

#include "Component.h"
#include <glm\glm.hpp>
#include <string>

namespace luna
{
	struct FontMaterial
	{
		eFONTS fontID = eFONTS::MAX_FONT;
		glm::vec4 color{};
		glm::vec4 outlinecolor{};
		glm::vec2 borderOffset{};
		float width = 0.f;
		float edge = 0.f;
		float borderwidth = 0.f;
		float borderedge = 0.f;
	};

	class FontComponent :
		public Component
	{
	public:
		FontComponent();
		virtual ~FontComponent();

		void Update() override;
		void Reset() override;

		std::string text = "text";
		FontMaterial material{};

	private:
		// only used for comparison in finding
		friend bool operator== (const FontComponent& n1, const FontComponent& n2);
	};
}

#endif // !FONT_COMPONENT_H

