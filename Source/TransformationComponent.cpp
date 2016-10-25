#include "TransformationComponent.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

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
			m_model = glm::translate(glm::mat4(), position) *
				glm::rotate(glm::mat4(), glm::radians(rotation.w), glm::vec3(rotation)) *
				glm::scale(glm::mat4(), scale);

			m_transpose_inverse_model = glm::transpose(glm::inverse(m_model));
		}
	}

	void TransformationComponent::Reset()
	{
		// reset everything 

		this->m_owner = nullptr;
		this->m_active = false;

		position = {};
		scale = glm::vec3(1.f, 1.f, 1.f);
		rotation = glm::vec4(0.f, 1.f, 0.f, 0.f);

		m_model = {};
		m_transpose_inverse_model = {};
	}

	bool operator==(const TransformationComponent & n1, const TransformationComponent & n2)
	{
		// if both pointing in the same address, then they are the same
		if (&n1 == &n2)
			return true;
		else
			return false;
		
	}
}