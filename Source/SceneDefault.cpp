#include "SceneDefault.h"
#include "ComponentManager.h"
#include "DebugLog.h"
#include "Renderer.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>
#include <glm\glm.hpp>

#include "WinNative.h"

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

	void SceneDefault::EarlyUpdate()
	{
		// every time when new entity is allocated with basicmesh to render, must renew the renderdatas again
		m_componentmanager->GetRenderingData(m_renderinfos);

		// update all the components locally
		m_componentmanager->Update();

		// prepare instance datas 
		std::vector<InstanceData> instancedatas;
		UpdateInstanceData_(instancedatas);

		// font data
		std::vector<FontInstanceData> fontinstancedatas;
		m_componentmanager->GetFontInstanceData(fontinstancedatas);

		UBOData data{};
		m_renderer->UploadDatas(data, instancedatas, fontinstancedatas);

		// rmb to re-record the command buffer again if m_renderinfos is different
		m_renderer->RecordGeometryPass(m_renderinfos);
	}

	void SceneDefault::Update()
	{
		// update all the components locally
		m_componentmanager->Update();

		// prepare rendering datas 
		// every frame gather all the transformation info for the mesh
		std::vector<InstanceData> instancedatas;
		UpdateInstanceData_(instancedatas);
		
		// view and projection mat4 update 
		WinNative* win = WinNative::getInstance();
		UBOData data{};
		data.view = glm::lookAt(glm::vec3(2.f, 2.f, -5.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		data.proj = glm::perspective(glm::radians(45.f), win->getWinSizeX() / static_cast<float>(win->getWinSizeY()), 0.1f, 10.0f);
		// invert y coordinate
		data.proj[1][1] *= -1.f;

		// +3 ms.. pls take note of this
		// this is expensive
		std::vector<FontInstanceData> fontinstancedatas;
		m_renderer->UploadDatas(data, instancedatas, fontinstancedatas);
	}

	void SceneDefault::Render()
	{
		m_renderer->Render();
	}

	void SceneDefault::LateUpdate()
	{
	}

	void SceneDefault::Init_()
	{
		// init all the components based on my max entities in the scene
		m_componentmanager = new ComponentManager(MAX_ENTITIES);

		// reserve all the entities in the vector 
		m_availableEntities.reserve(MAX_ENTITIES);

		// entity example init .. will use lua script in the future

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("first", m_componentmanager);
			entity->transformation->position = glm::vec3(2.f, 0, 1.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::CUBE_MODEL;
			basicmeshc->material.color = glm::vec4(1.f, 0.f, 0.f, 0.f);
			basicmeshc->material.textureID = eTEXTURES::BASIC_2D_BC2;
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("second", m_componentmanager);
			entity->transformation->position = glm::vec3(0.f, 2.f, -2.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::BUNNY_MODEL;
			basicmeshc->material.color = glm::vec4(0.f, 1.f, 0.f, 0.f);
			basicmeshc->material.textureID = eTEXTURES::BLACK_2D_RGBA;
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("second", m_componentmanager);
			entity->transformation->position = glm::vec3(1.f, 1.f, -1.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::BUNNY_MODEL;
			basicmeshc->material.color = glm::vec4(1.f, 0.f, 0.f, 0.f);
			basicmeshc->material.textureID = eTEXTURES::BLACK_2D_RGBA;
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("third", m_componentmanager);
			entity->transformation->position = glm::vec3(-1.f, -2.f, -1.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::CUBE_MODEL;
			basicmeshc->material.color = glm::vec4(0.f, 0.f, 0.f, 0.f);
			basicmeshc->material.textureID = eTEXTURES::BASIC_2D_BC2;
			m_availableEntities.push_back(entity);
		}
		
		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("font", m_componentmanager);
			entity->transformation->position = glm::vec3(6.f, 6.f, -6.f);
			FontComponent* fontc = dynamic_cast<FontComponent*>(entity->AddComponent(COMPONENT_ATYPE::FONT_ACTYPE));
			fontc->material.fontID = FONT_EVA;
			fontc->text = "NEON GENESIS EVANGELION";
			fontc->material.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
			fontc->material.outlinecolor = glm::vec4(0.55f, 0.23f, 0.1f, 0.f);
			fontc->material.borderOffset = glm::vec2(0.0015f, 0.000f);
			fontc->material.width = 0.45f;
			fontc->material.edge = 0.15f;
			fontc->material.borderwidth = 0.5f;
			fontc->material.borderedge = 0.15f;
			m_availableEntities.push_back(entity);
		}
	}

	void SceneDefault::DeInit_()
	{
		
	}
}
