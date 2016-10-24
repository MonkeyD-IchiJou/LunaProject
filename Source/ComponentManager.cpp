#include "ComponentManager.h"

namespace luna
{
	ComponentManager::ComponentManager(const int MAX_COMPONENTS)
	{
		/* Init and resize all the components */
		m_transformationContainer.components.resize(MAX_COMPONENTS);
		m_basicmeshContainer.components.resize(MAX_COMPONENTS);
		m_fontContainer.components.resize(MAX_COMPONENTS);
	}

	ComponentManager::~ComponentManager()
	{
		/* delete all the components */
	}

	void ComponentManager::Update()
	{
		m_transformationContainer.Update();
		m_basicmeshContainer.Update();
		m_fontContainer.Update();
	}
}
