#ifndef COMPONENT_MANAGER_H
#define COMPONENT_MANAGER_H

#include "TransformationComponent.h"
#include "FontComponent.h"
#include "BasicMeshComponent.h"
#include "CameraComponent.h"
#include "ScriptComponent.h"

#include "FramePacket.h"
#include "DebugLog.h"

#include <typeinfo>
#include <vector>
#include <algorithm>

namespace luna
{
	template <typename T>
	class ComponentData
	{
	public:
		void Update()
		{
			// update all the components locally
			for (auto & component : m_components)
			{
				component.Update();
			}
		}

		T* GetComponent()
		{
			for (auto & component : m_components)
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
		std::vector<T> m_components;

		/* only component manager can init my components and access my private members */
		friend class ComponentManager; 
	};

	class ComponentManager
	{
	public:
		ComponentManager(const int MAX_COMPONENTS);
		~ComponentManager();

		/* update all the components */
		void Update();

		/* fill up the rendering datas */
		void GetRenderingData(std::vector<RenderingInfo>& renderinfos);

		/* fill up the fonts instance data */
		void GetFontInstanceData(std::vector<FontInstanceData>& fontinstancedatas);

		/* get main camera data */
		void GetMainCamData(UBOData& maincamdata);

	private:
		static void StoreNewRenderingData_(BasicMeshComponent* mesh, std::vector<RenderingInfo>& renderdatas, RenderingInfo& rd);
		static void StoreRenderingData_(BasicMeshComponent* mesh, RenderingInfo& rd);

	private:
		/* all the transformation components are here */
		ComponentData<TransformationComponent> m_transformationContainer;
		
		/* all the Basic Mesh components are here */
		ComponentData<BasicMeshComponent> m_basicmeshContainer;

		/* all the font components are here */
		ComponentData<FontComponent> m_fontContainer;

		/* all the camera components are here */
		ComponentData<CameraComponent> m_cameraContainer;

		/* all the script components are here */
		ComponentData<ScriptComponent> m_scriptContainer;

		/* only entity class can access my components freely */
		friend class Entity;
	};
}

#endif

