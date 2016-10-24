#ifndef SCENE_SMALL_H
#define SCENE_SMALL_H

#include "SceneVolume.h"
#include <array>

namespace luna
{
	class SceneMedium : public SceneVolume
	{
	protected:
		SceneMedium();
		virtual ~SceneMedium();

		/* The max entity allowed in a medium scene */
		const static int MAX_ENTITIES = 100;

		/* find the entity which is not awakened */
		Entity* GetAvailableEntity_() override;

	private:
		/* all entities are here, every scenes have its own total num of entity */
		std::array<Entity, MAX_ENTITIES> m_entities{};
	};
}

#endif
