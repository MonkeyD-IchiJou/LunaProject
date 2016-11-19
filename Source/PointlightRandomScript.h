#ifndef POINTLIGHT_RANDOM_SCRIPT_H
#define POINTLIGHT_RANDOM_SCRIPT_H

#include "Script.h"
#include <glm/glm.hpp>

namespace luna
{
	class PointlightRandomScript :
		public Script
	{
	public:
		PointlightRandomScript();
		virtual ~PointlightRandomScript();

		void Update(Entity* entity) override;

	private:
		bool firstInit = false;
		glm::vec3 center = {0.f, 2.0f, 0.f};
		glm::vec3 angle = {};
		glm::vec3 radius = {7.6f, 6.6f, 7.5f};
		glm::vec3 speed = { 2.1f, 1.5f, 1.85f };
	};
}

#endif

