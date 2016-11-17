#include "PointLightComponent.h"

namespace luna
{
	PointLightComponent::PointLightComponent() : Component(POINTLIGHT_CTYPE)
	{
	}

	PointLightComponent::~PointLightComponent()
	{
	}

	void PointLightComponent::Update()
	{
	}

	void PointLightComponent::Reset()
	{
		this->m_owner = nullptr;
		this->m_active = false;

		color = {};
	}

}
