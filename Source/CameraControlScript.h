#ifndef CAMERA_CONTROL_SCRIPT_H
#define CAMERA_CONTROL_SCRIPT_H

#include "Script.h"

#include <glm/glm.hpp>

namespace luna
{
	class CameraControlScript :
		public Script
	{
	public:
		CameraControlScript();
		virtual ~CameraControlScript();
		void Update(Entity* entity) override;

	private:
		glm::vec2 Angle{ 0.0f };
		glm::vec2 mouseSpeed{ -0.005f, 0.005f };
		glm::vec2 currentmousepos{};
		glm::vec2 prevmousepos{};
		float magnitude = 55.f;
		float speed = 3.0f; // 3 units / second
		bool firstclick = true;
	};
}

#endif

