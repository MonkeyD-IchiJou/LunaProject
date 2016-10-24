#ifndef COMPONENT_H
#define COMPONENT_H

#include "enum_c.h"

namespace luna
{
	class Entity;

	class Component
	{
	public:
		/* update the component if necessary */
		virtual void Update() = 0;

		/* reset the component */
		virtual void Reset() = 0;

		inline bool isActive() const { return m_active; }
		inline void SetActive(const bool& active) { this->m_active = active; }
		inline void SetOwner(Entity* entity) { this->m_owner = entity; }
		inline Entity* GetOwner() { return this->m_owner; }

	protected:
		Component(const COMPONENT_TYPE& component_type);
		virtual ~Component();

		/* what is the type of this component */
		COMPONENT_TYPE m_componenttype = LAST_CTYPE;

		/* the active state of this component, if is inactive, it will not update itself */
		bool m_active = false;

		/* the entity that is owning this component */
		Entity* m_owner = nullptr;
	};
}

#endif // !COMPONENT_H


