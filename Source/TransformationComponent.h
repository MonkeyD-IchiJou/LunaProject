#ifndef TRANSFORMATION_COMPONENT_H
#define TRANSFORMATION_COMPONENT_H

#include "Component.h"

#include <glm\glm.hpp>

namespace luna
{
	class TransformationComponent :
		public Component
	{
	public:
		TransformationComponent();
		virtual ~TransformationComponent();

		void Update() override;
		void Reset() override;

	public:
		glm::vec3 position{};
		glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f);
		glm::vec4 rotation = glm::vec4(0.f, 1.f, 0.f, 0.f);

	private:
		glm::mat4 m_model = {};
		glm::mat4 m_transpose_inverse_model = {};
	};
}

#endif // !TRANSFORMATION_COMPONENT_H

