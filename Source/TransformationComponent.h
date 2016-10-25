#ifndef TRANSFORMATION_COMPONENT_H
#define TRANSFORMATION_COMPONENT_H

#include "Component.h"

#include <glm\glm.hpp>
#include <iostream>

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

		auto GetModel() const { return m_model; }
		auto GetTransposeInverseModel() const { return m_transpose_inverse_model; }

	public:
		glm::vec3 position{};
		glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f);
		glm::vec4 rotation = glm::vec4(0.f, 1.f, 0.f, 0.f);

	private:
		glm::mat4 m_model = {};
		glm::mat4 m_transpose_inverse_model = {};

		// only used for comparison in finding
		friend bool operator== (const TransformationComponent& n1, const TransformationComponent& n2);
	};
}

#endif // !TRANSFORMATION_COMPONENT_H

