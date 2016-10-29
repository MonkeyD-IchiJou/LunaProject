#include "SceneDefault.h"
#include "ComponentManager.h"
#include "DebugLog.h"
#include "Renderer.h"

#include "RotateScript.h"
#include <glm\glm.hpp>

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

	void SceneDefault::Update(FramePacket& framepacket, std::array<JobSystem, 3>& workers)
	{
		/* prepare Frame Packets datas !! */ 

		/* update all the components locally */
		m_componentmanager->Update();

		// every time when new entity is allocated with basicmesh to render, must renew the renderdatas again
		// every frame gather all the transformation info for the mesh
		workers[0].addJob([&]() {
			m_componentmanager->GetRenderingData(framepacket.renderinfos);
			GetInstanceData_(framepacket.instancedatas, framepacket.renderinfos);
		});
		
		// gather font data
		workers[1].addJob([&]() {
			m_componentmanager->GetFontInstanceData(framepacket.fontinstancedatas);
			m_componentmanager->GetMainCamData(framepacket.maincamdata);
		});

		// let all workers finish their job pls
		workers[0].wait();
		workers[1].wait();
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
			basicmeshc->material.textureID = MESH_TEX::BOX_TEX;
			ScriptComponent* script = dynamic_cast<ScriptComponent*>(entity->AddComponent(COMPONENT_ATYPE::SCRIPT_ACTYPE));
			script->script = new RotateScript();
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("second", m_componentmanager);
			entity->transformation->position = glm::vec3(0.f, 2.f, -2.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::BUNNY_MODEL;
			basicmeshc->material.color = glm::vec4(0.f, 1.f, 0.f, 0.f);
			basicmeshc->material.textureID = MESH_TEX::BLACK_TEX;
			ScriptComponent* script = dynamic_cast<ScriptComponent*>(entity->AddComponent(COMPONENT_ATYPE::SCRIPT_ACTYPE));
			script->script = new RotateScript();
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("second", m_componentmanager);
			entity->transformation->position = glm::vec3(1.f, 1.f, -1.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::BUNNY_MODEL;
			basicmeshc->material.color = glm::vec4(1.f, 0.f, 0.f, 0.f);
			basicmeshc->material.textureID = MESH_TEX::BLACK_TEX;
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("third", m_componentmanager);
			entity->transformation->position = glm::vec3(-1.f, -2.f, -1.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::CUBE_MODEL;
			basicmeshc->material.color = glm::vec4(0.f, 0.f, 0.f, 0.f);
			basicmeshc->material.textureID = MESH_TEX::BOX_TEX;
			m_availableEntities.push_back(entity);
		}
		
		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("font", m_componentmanager);
			entity->transformation->position = glm::vec3(705.f, 100.f, 0.f);
			entity->transformation->scale = glm::vec3(400.f, 400.f, 1.f);
			FontComponent* fontc = dynamic_cast<FontComponent*>(entity->AddComponent(COMPONENT_ATYPE::FONT_ACTYPE));
			fontc->material.fontID = FONT_EVA;
			fontc->text = "NEON GENESIS EVAGENLION";
			fontc->material.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
			fontc->material.outlinecolor = glm::vec4(0.f, 0.f, 0.0f, 0.f);
			fontc->material.borderOffset = glm::vec2(0.0015f, 0.000f);
			fontc->material.width = 0.45f;
			fontc->material.edge = 0.15f;
			fontc->material.borderwidth = 0.5f;
			fontc->material.borderedge = 0.15f;
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("font", m_componentmanager);
			entity->transformation->position = glm::vec3(5.f, 720.f, 0.f);
			entity->transformation->scale = glm::vec3(400.f, 400.f, 1.f);
			FontComponent* fontc = dynamic_cast<FontComponent*>(entity->AddComponent(COMPONENT_ATYPE::FONT_ACTYPE));
			fontc->material.fontID = FONT_EVA;
			fontc->text = "NEON GENESIS EVAGENLION";
			fontc->material.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
			fontc->material.outlinecolor = glm::vec4(0.75f, 0.55f, 0.1f, 0.f);
			fontc->material.borderOffset = glm::vec2(0.0015f, 0.000f);
			fontc->material.width = 0.45f;
			fontc->material.edge = 0.15f;
			fontc->material.borderwidth = 0.5f;
			fontc->material.borderedge = 0.15f;
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("main camera", m_componentmanager);
			entity->transformation->position = glm::vec3(-2.f, 2.f, -5.f);
			CameraComponent* camc = dynamic_cast<CameraComponent*>(entity->AddComponent(COMPONENT_ATYPE::CAMERA_ACTYPE));
			camc->maincam = true;
			m_availableEntities.push_back(entity);
		}
	}

	void SceneDefault::DeInit_()
	{
		
	}
}
