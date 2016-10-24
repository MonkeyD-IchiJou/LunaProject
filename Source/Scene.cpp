#include "Scene.h"
#include "ComponentManager.h"

namespace luna
{
	Scene::Scene()
	{
	}

	Scene::~Scene()
	{
		if (m_componentmanager != nullptr)
		{
			delete m_componentmanager;
			m_componentmanager = nullptr;
		}
	}
}