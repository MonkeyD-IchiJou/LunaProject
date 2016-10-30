#ifndef ENTITY_H
#define ENTITY_H

#include <vector>
#include <string>
#include "enum_c.h"

namespace luna
{
	class ComponentManager;
	class TransformationComponent;
	class Component;

	class Entity
	{
	public:
		/* only when entity is awake, then can add components to it */
		void Awake(const std::string& name, ComponentManager* componentManager);

		/* reset/sleep the entity and reset the components */
		void Reset();

		/* check is awakened liao mah */
		inline bool isAwake() const { return m_awake; }

		/* check is active or not */
		inline bool isActive() const { return m_active; }

		/* set the active state of the entity */
		void SetActive(const bool& active);

		/* Add the available/accepted component, then return a handle of it */
		Component* AddComponent(const COMPONENT_ATYPE& componentType);

		/* find the specific component, and then return the original class type of it */
		template<typename T>
		T * findComponentT(void)
		{
			for (auto &x : m_componentsContainer)
			{
				if (T* temp = dynamic_cast<T*>(x))
				{
					return temp;
				}
			}

			// if cannot find it
			return nullptr;
		}

		/* default destructor */
		~Entity();

	private:
		/* setup & register the component into the container */
		Component* RegisterComponent_(Component* component);

		/* reset and remove the component from the container */
		void RemoveComponent_(Component* component);

		/* default constructor */
		Entity();

		/* only scene (different volume) can init my entity */
		friend class SceneSmall;
		friend class SceneMedium;
		friend class SceneLarge;

	public:
		/* every entity must have a transformation component by default */
		TransformationComponent* transformation = nullptr;

	private:
		/* every entity have a name */
		std::string m_name = "default";

		/* the entity is awake or not, entity is not allowed to add components when sleeping */
		bool m_awake = false;

		/* active or not will affect the components update */
		bool m_active = false;

		/* every entity can have different kind of components to define itself */
		std::vector<Component*> m_componentsContainer;

		/* component manager to retrieve components that i needed */
		ComponentManager* m_componentManager = nullptr;
	};
}

#endif

