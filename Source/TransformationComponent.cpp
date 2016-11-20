#include "TransformationComponent.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

namespace luna
{
	TransformationComponent::TransformationComponent() : Component(TRANSFORMATION_CTYPE)
	{
	}

	TransformationComponent::~TransformationComponent()
	{
	}

	void TransformationComponent::Update()
	{
		// only update the component when is active
		if (m_active)
		{
			// store the prev model
			m_prevmodel = m_model;

			// current model update
			m_model = glm::translate(glm::mat4(), position) *
				glm::toMat4(glm::quat(glm::radians(eulerangles))) *
				glm::scale(glm::mat4(), scale);
		}
	}

	void TransformationComponent::Reset()
	{
		// reset everything 

		this->m_owner = nullptr;
		this->m_active = false;

		position = {};
		scale = glm::vec3(1.f, 1.f, 1.f);
		eulerangles = glm::vec4(0.f, 1.f, 0.f, 0.f);

		m_model = {};
	}
}
