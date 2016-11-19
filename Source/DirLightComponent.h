#ifndef DIR_LIGHT_COMPONENT_H
#define DIR_LIGHT_COMPONENT_H

#include "Component.h"

#include <glm/glm.hpp>

namespace luna
{
	class DirLightComponent :
		public Component
	{
	public:
		DirLightComponent();
		virtual ~DirLightComponent();

		void Update() override;
		void Reset() override;

		glm::vec3 direction = {0.f, -1.f, 0.f};
		glm::vec3 diffuse = { 0.5f, 0.5f, 0.5f };
		glm::vec3 ambient = { 0.01f, 0.01f, 0.01f };
		float specular = 0.4f;
	};
}

#endif

