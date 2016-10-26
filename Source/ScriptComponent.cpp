#include "ScriptComponent.h"

#include "Entity.h"
#include "TransformationComponent.h"
#include <glm\glm.hpp>

namespace luna
{
	ScriptComponent::ScriptComponent() : Component(SCRIPT_TYPE)
	{
	}

	ScriptComponent::~ScriptComponent()
	{
	}

	void ScriptComponent::Update()
	{
		// only update the component when is active
		if (m_active)
		{
			auto t = m_owner->transformation;
			t->rotation.x = 0.f; t->rotation.y = 1.f; t->rotation.x = 0.f;
			t->rotation.w += 0.1f;
		}
	}

	void ScriptComponent::Reset()
	{
		this->m_owner = nullptr;
		this->m_active = false;
	}
}
