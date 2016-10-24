#include "Entity.h"
#include "DebugLog.h"
#include "ComponentManager.h"

namespace luna
{
	Entity::Entity()
	{
	}

	Entity::~Entity()
	{
	}

	void Entity::Awake(const std::string& name, ComponentManager* componentManager)
	{
		m_name = name;
		m_componentManager = componentManager;

		if (m_componentManager == nullptr)
			DebugLog::throwEx("component manager not available");

		// if is not awake
		if (!m_awake)
		{
			m_awake = true;
			m_active = true;

			// auto register the transformation component
			transformation = dynamic_cast<TransformationComponent*>(m_componentManager->GetComponent<TransformationComponent>());
			RegisterComponent_(transformation);
		}
	}

	void Entity::Reset()
	{
		m_awake = false;
		m_active = false;
		m_name = "default";

		m_componentManager = nullptr;

		// reset all the components 
		for (auto &c : m_componentsContainer)
		{
			if (c != nullptr)
			{
				c->Reset();
			}
		}
		m_componentsContainer.clear();
	}

	void Entity::SetActive(const bool & active)
	{
		this->m_active = active;

		for (auto &c : m_componentsContainer)
		{
			if (c != nullptr)
			{
				c->SetActive(active);
			}
		}
	}

	Component * Entity::AddComponent(const COMPONENT_ATYPE & componentType)
	{
		// only when i awake then can add component
		if (m_awake)
		{
			if (m_componentManager == nullptr)
			{
				DebugLog::throwEx("component manager not available");
			}

			Component* component = nullptr;

			switch (componentType)
			{
			case COMPONENT_ATYPE::BASICMESH_ACTYPE:
				component = m_componentManager->GetComponent<BasicMeshComponent>();
				break;

			case COMPONENT_ATYPE::FONT_ACTYPE:
				component = m_componentManager->GetComponent<FontComponent>();
				break;

			default:
				break;
			}

			return RegisterComponent_(component);
		}
		else
		{
			DebugLog::throwEx("entity is not awaken yet, cannot add component");
			return nullptr;
		}
	}

	Component * Entity::RegisterComponent_(Component * component)
	{
		// only when i awake then can register
		if (m_awake)
		{
			// if dun have the component
			if (component == nullptr)
				DebugLog::throwEx("component not available ! ");

			// set the active state same as the entity
			component->SetActive(m_active);

			// if the component has alr been registered
			if(component->GetOwner() != nullptr)
				DebugLog::throwEx("component has alr been registered, are you sured about it ! ");

			// set the component owner (registered)
			component->SetOwner(this);

			// push the component back to the container
			m_componentsContainer.push_back(component);

			// return the component back to the user
			return m_componentsContainer.back();
		}
		else
		{
			DebugLog::throwEx("entity is not awaken yet, cannot register component");
			return nullptr;
		}
	}
}