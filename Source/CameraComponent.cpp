#include "CameraComponent.h"
#include "WinNative.h"
#include "Entity.h"
#include "TransformationComponent.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

namespace luna
{
	CameraComponent::CameraComponent() : Component(CAMERA_CTYPE)
	{
	}

	CameraComponent::~CameraComponent()
	{
	}

	void CameraComponent::Update()
	{
		if (m_active)
		{
			// store the prev proj * view mat4
			m_prevview = m_view;

			auto position = m_owner->transformation->position;
			m_view = glm::lookAt(position, target, up);

			WinNative* win = WinNative::getInstance();
			m_projection = glm::perspective(glm::radians(fovy), win->getWinSurfaceSizeX() / static_cast<float>(win->getWinSurfaceSizeY()), nearZ, farZ);
			m_projection[1][1] *= -1.f;
		}
	}

	void CameraComponent::Reset()
	{
		this->m_owner = nullptr;
		this->m_active = false;

		target = {};
		up = {};

		fovy = 45.f;
		nearZ = 0.1f;
		farZ = 10.f;

		m_view = {};
		m_projection = {};
	}
}
