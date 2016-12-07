#include "SceneDefault.h"
#include "ComponentManager.h"

#include "RotateScript.h"
#include "TextChangeScript.h"
#include "TextChangeScript2.h"
#include "CameraControlScript.h"
#include "PointlightRandomScript.h"

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

	void SceneDefault::Update(FramePacket& framepacket, std::array<Worker*, 2>& workers)
	{
		/* prepare Frame Packets datas !! */ 

		/* update all the components locally */
		m_componentmanager->Update();

		// every time when new entity is allocated with basicmesh to render, must renew the renderdatas again
		// every frame gather all the transformation info for the mesh

		// first worker
		workers[0]->addJob([&]() {
			framepacket.renderinfos = &m_renderinfos;
			GetInstanceData_(framepacket.instancedatas, m_renderinfos);
		});
		
		// second worker 
		workers[1]->addJob([&]() {
			m_componentmanager->GetFontInstanceData(framepacket.fontinstancedatas);
			m_componentmanager->GetMainCamData(framepacket.maincamdata, framepacket.maincampos);
			m_componentmanager->GetMainDirLightData(framepacket.dirlightdata);
			m_componentmanager->GetPointLightsData(framepacket.pointlightsdatas);
		});

		// make sure the two workers have finish all their prev jobs
		workers[0]->wait();
		workers[1]->wait();
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
			entity->Awake("platform cube", m_componentmanager);
			entity->transformation->position = glm::vec3(0.f, 4.f, 0.f);
			entity->transformation->scale = glm::vec3(20.f, 20.f, 20.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::SKYBOX_MODEL;
			basicmeshc->material.color = glm::vec4(1.f, 1.f, 1.f, 2.f);
			basicmeshc->material.textureID = MESH_TEX::BLACK_TEX;
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("bunny", m_componentmanager);
			entity->transformation->position = glm::vec3(4.f, -1.f, -6.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::BUNNY_MODEL;
			basicmeshc->material.color = glm::vec4(0.1f, 1.f, 0.2f, 0.f);
			basicmeshc->material.textureID = MESH_TEX::BLACK_TEX;
			ScriptComponent* script = dynamic_cast<ScriptComponent*>(entity->AddComponent(COMPONENT_ATYPE::SCRIPT_ACTYPE));
			script->script = new RotateScript();
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("box", m_componentmanager);
			entity->transformation->position = glm::vec3(8.f, -1.f, -8.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::CUBE_MODEL;
			basicmeshc->material.color = glm::vec4(0.f, 0.f, 0.f, 2.f);
			basicmeshc->material.textureID = MESH_TEX::BOX_TEX;
			ScriptComponent* script = dynamic_cast<ScriptComponent*>(entity->AddComponent(COMPONENT_ATYPE::SCRIPT_ACTYPE));
			script->script = new RotateScript();
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("box", m_componentmanager);
			entity->transformation->position = glm::vec3(0.f, 0.f, 8.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::CUBE_MODEL;
			basicmeshc->material.color = glm::vec4(0.f, 0.f, 0.f, 2.f);
			basicmeshc->material.textureID = MESH_TEX::BOX_TEX;
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("suzanna", m_componentmanager);
			entity->transformation->position = glm::vec3(-5.f, -1.f, -8.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::SUZANNA_MODEL;
			basicmeshc->material.color = glm::vec4(0.7f, 0.7f, 0.2f, 0.f);
			basicmeshc->material.textureID = MESH_TEX::BLACK_TEX;
			ScriptComponent* script = dynamic_cast<ScriptComponent*>(entity->AddComponent(COMPONENT_ATYPE::SCRIPT_ACTYPE));
			script->script = new RotateScript();
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("dragon", m_componentmanager);
			entity->transformation->position = glm::vec3(0.f, -5.5f, 0.f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::DRAGON_MODEL;
			basicmeshc->material.color = glm::vec4(1.f, 0.25f, 0.5f, 0.f);
			basicmeshc->material.textureID = MESH_TEX::BLACK_TEX;
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("sphere", m_componentmanager);
			entity->transformation->position = glm::vec3(-5.5f, -2.f, 5.5f);
			BasicMeshComponent* basicmeshc = dynamic_cast<BasicMeshComponent*>(entity->AddComponent(COMPONENT_ATYPE::BASICMESH_ACTYPE));
			basicmeshc->meshID = eMODELS::SPHERE_MODEL;
			basicmeshc->material.color = glm::vec4(0.1f, 0.1f, 1.f, 0.f);
			basicmeshc->material.textureID = MESH_TEX::BLACK_TEX;
			m_availableEntities.push_back(entity);
		}
		
		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("font", m_componentmanager);
			entity->transformation->position = glm::vec3(705.f, 60.f, 0.f);
			entity->transformation->scale = glm::vec3(400.f, 400.f, 1.f);
			FontComponent* fontc = dynamic_cast<FontComponent*>(entity->AddComponent(COMPONENT_ATYPE::FONT_ACTYPE));
			fontc->material.fontID = FONT_EVA;
			fontc->text = "NEON GENESIS EVAGENLION";
			fontc->material.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
			fontc->material.outlinecolor = glm::vec4(0.f, 0.f, 0.0f, 0.f);
			fontc->material.borderOffset = glm::vec2(0.0f, 0.0f);
			fontc->material.width = 0.45f;
			fontc->material.edge = 0.15f;
			fontc->material.borderwidth = 0.53f;
			fontc->material.borderedge = 0.15f;
			ScriptComponent* script = dynamic_cast<ScriptComponent*>(entity->AddComponent(COMPONENT_ATYPE::SCRIPT_ACTYPE));
			script->script = new TextChangeScript2();
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("font", m_componentmanager);
			entity->transformation->position = glm::vec3(5.f, 0.f, 0.f);
			entity->transformation->scale = glm::vec3(400.f, 400.f, 1.f);
			FontComponent* fontc = dynamic_cast<FontComponent*>(entity->AddComponent(COMPONENT_ATYPE::FONT_ACTYPE));
			fontc->material.fontID = FONT_EVA;
			fontc->text = "NEON GENESIS EVAGENLION";
			fontc->material.color = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
			fontc->material.outlinecolor = glm::vec4(0.75f, 0.55f, 0.1f, 0.f);
			fontc->material.borderOffset = glm::vec2(0.0f, 0.0f);
			fontc->material.width = 0.45f;
			fontc->material.edge = 0.15f;
			fontc->material.borderwidth = 0.5f;
			fontc->material.borderedge = 0.15f;
			ScriptComponent* script = dynamic_cast<ScriptComponent*>(entity->AddComponent(COMPONENT_ATYPE::SCRIPT_ACTYPE));
			script->script = new TextChangeScript();
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("main camera", m_componentmanager);
			entity->transformation->position = glm::vec3(-15.f, 5.f, -15.f);
			CameraComponent* camc = dynamic_cast<CameraComponent*>(entity->AddComponent(COMPONENT_ATYPE::CAMERA_ACTYPE));
			camc->maincam = true;
			camc->farZ = 200.f;
			ScriptComponent* script = dynamic_cast<ScriptComponent*>(entity->AddComponent(COMPONENT_ATYPE::SCRIPT_ACTYPE));
			script->script = new CameraControlScript();
			m_availableEntities.push_back(entity);
		}

		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("dirlight", m_componentmanager);
			DirLightComponent* dirlightc = dynamic_cast<DirLightComponent*>(entity->AddComponent(COMPONENT_ATYPE::DIRLIGHT_ACTYPE));
			dirlightc->direction = glm::vec3(0.5f, -0.5f, 1.0f);
			dirlightc->diffuse = glm::vec3( 0.5f, 0.5f, 0.5f);
			dirlightc->ambient = glm::vec3(0.01f, 0.01f, 0.01f);
			dirlightc->specular = 0.5f;
			m_availableEntities.push_back(entity);
		}

		// all point lights below
		for(int i = 0; i < 40; ++i)
		{
			Entity* entity = GetAvailableEntity_();
			entity->Awake("pointlight", m_componentmanager);
			entity->AddComponent(COMPONENT_ATYPE::POINTLIGHT_ACTYPE);
			ScriptComponent* script = dynamic_cast<ScriptComponent*>(entity->AddComponent(COMPONENT_ATYPE::SCRIPT_ACTYPE));
			script->script = new PointlightRandomScript();
			m_availableEntities.push_back(entity);
		}

		// if have new entities with mesh info, need to renew this again
		m_componentmanager->GetRenderingData(m_renderinfos);
	}

	void SceneDefault::DeInit_()
	{
		
	}
}
