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
		glm::vec3 dir = {};
	};
}

#endif

