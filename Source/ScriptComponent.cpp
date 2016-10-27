#include "ScriptComponent.h"

#include "Script.h"

namespace luna
{
	ScriptComponent::ScriptComponent() : Component(SCRIPT_TYPE)
	{
	}

	ScriptComponent::~ScriptComponent()
	{
		if (script != nullptr)
		{
			delete script;
			script = nullptr;
		}
	}

	void ScriptComponent::Update()
	{
		// only update the component when is active
		if (m_active)
		{
			script->Update(this->GetOwner());
		}
	}

	void ScriptComponent::Reset()
	{
		this->m_owner = nullptr;
		this->m_active = false;

		if (script != nullptr)
		{
			delete script;
			script = nullptr;
		}
	}
}
