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

		auto GetModel() const { return m_model; }
		auto GetTransposeInverseModel() const { return m_transpose_inverse_model; }

	public:
		glm::vec3 position{};
		glm::vec3 scale = glm::vec3(1.f, 1.f, 1.f);
		glm::vec3 eulerangles = glm::vec3(0.f, 0.f, 0.f);

	private:
		glm::mat4 m_model = {};
		glm::mat4 m_transpose_inverse_model = {};
	};
}

#endif // !TRANSFORMATION_COMPONENT_H

