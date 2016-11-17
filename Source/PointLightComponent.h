#ifndef POINT_LIGHT_COMPONENT_H
#define POINT_LIGHT_COMPONENT_H

#include "Component.h"
#include <glm/glm.hpp>
namespace luna
{
	class PointLightComponent :
		public Component
	{
	public:
		PointLightComponent();
		virtual ~PointLightComponent();

		void Update() override;
		void Reset() override;

		glm::vec3 color = {};
	};
}

#endif

