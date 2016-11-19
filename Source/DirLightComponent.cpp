#include "DirLightComponent.h"


namespace luna
{
	DirLightComponent::DirLightComponent() : Component(DIRLIGHT_CTYPE)
	{
	}

	DirLightComponent::~DirLightComponent()
	{
	}

	void DirLightComponent::Update()
	{
	}

	void DirLightComponent::Reset()
	{
		// reset everything 

		this->m_owner = nullptr;
		this->m_active = false;

		direction = {0.f, -1.f, 0.f};
		diffuse = { 0.5f, 0.5f, 0.5f };
		ambient = { 0.01f, 0.01f, 0.01f };
		specular = 0.4f;
	}
}
