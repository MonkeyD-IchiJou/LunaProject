#ifndef COMPONENT_MANAGER_H
#define COMPONENT_MANAGER_H

#include "TransformationComponent.h"
#include "FontComponent.h"
#include "BasicMeshComponent.h"

#include "DebugLog.h"

#include <typeinfo>
#include <vector>

namespace luna
{
	template <typename T>
	class ComponentData
	{
	public:
		void Update()
		{
			// update all the components locally
			for (auto &component : components )
			{
				component.Update();
			}
		}

		T* GetComponent()
		{
			for ( auto &component : components )
			{
				// check which components do not have any owner, then return the one
				if (component.GetOwner() == nullptr)
				{
					return &component;
				}
			}

			// if cannot find any, return the nullptr to it (full liao)
			DebugLog::throwEx("components full liao");
			return nullptr;
		}

	private:
		/* all the components data are here */
		std::vector<T> components;

		/* only component manager can init my components */
		friend class ComponentManager; 
	};

	class ComponentManager
	{
	public:
		ComponentManager(const int MAX_COMPONENTS);
		~ComponentManager();

		/* update all the components */
		void Update();

		/* get certain components */
		template <typename T>
		Component* GetComponent()
		{
			// if is a certain component type
			if (typeid(T) == typeid(TransformationComponent))
			{
				return m_transformationContainer.GetComponent();
			}
			else if (typeid(T) == typeid(BasicMeshComponent))
			{
				return m_basicmeshContainer.GetComponent();
			}
			else if (typeid(T) == typeid(FontComponent))
			{
				return m_fontContainer.GetComponent();
			}

			// if cannot find anything
			return nullptr;
		}

	private:
		/* all the transformation components are here */
		ComponentData<TransformationComponent> m_transformationContainer;
		
		/* all the Basic Mesh components are here */
		ComponentData<BasicMeshComponent> m_basicmeshContainer;

		/* all the font components are here */
		ComponentData<FontComponent> m_fontContainer;
	};
}

#endif

