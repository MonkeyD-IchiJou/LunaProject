#include "SceneDefault.h"
#include "ComponentManager.h"
#include "DebugLog.h"

namespace luna
{
	SceneDefault::SceneDefault()
	{
		// auto init
		Init_();
	}

	SceneDefault::~SceneDefault()
	{
		DeInit_();
	}

	void SceneDefault::Update()
	{
		// update all the components locally
		m_componentmanager->Update();
	}

	void SceneDefault::Render()
	{
	}

	void SceneDefault::LateUpdate()
	{
	}

	void SceneDefault::Init_()
	{
		// init all the components based on my max entities in the scene
		m_componentmanager = new ComponentManager(MAX_ENTITIES);
		
		{
			Entity& entity = *GetAvailableEntity_();
			entity.Awake("first", m_componentmanager);
			entity.transformation->position = glm::vec3(2.f, 0, 1.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity.AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::CUBE_MODEL;
			basicmeshc->material.color = glm::vec4(0.f);
			basicmeshc->material.textureID = eTEXTURES::BASIC_2D_BC2;
		}

		{
			Entity& entity = *GetAvailableEntity_();
			entity.Awake("second", m_componentmanager);
			entity.transformation->position = glm::vec3(0.f, 2.f, -2.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity.AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::BUNNY_MODEL;
			basicmeshc->material.color = glm::vec4(1.f, 0.f, 0.f, 0.f);
			basicmeshc->material.textureID = eTEXTURES::BASIC_2D_RGBA8;
		}
		
		{
			Entity& entity = *GetAvailableEntity_();
			entity.Awake("font", m_componentmanager);
			FontComponent* fontc = dynamic_cast<FontComponent*>(entity.AddComponent(COMPONENT_ATYPE::FONT_ACTYPE));
			fontc->text = "NEON GENESIS EVANGELION";
			fontc->material.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
			fontc->material.outlinecolor = glm::vec4(0.55f, 0.23f, 0.1f, 0.f);
			fontc->material.borderOffset = glm::vec2(0.0015f, 0.000f);
			fontc->material.width = 0.45f;
			fontc->material.edge = 0.15f;
			fontc->material.borderwidth = 0.5f;
			fontc->material.borderedge = 0.15f;
		}
	}

	void SceneDefault::DeInit_()
	{
		
	}
}
