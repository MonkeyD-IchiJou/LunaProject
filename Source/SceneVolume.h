#ifndef SCENE_VOLUME_H
#define SCENE_VOLUME_H

#include "Entity.h"

namespace luna
{
	class SceneVolume
	{
	protected:
		SceneVolume();
		virtual ~SceneVolume();

		virtual	Entity* GetAvailableEntity_() = 0;
		std::vector<Entity*> m_availableEntities;
	};
}

#endif

