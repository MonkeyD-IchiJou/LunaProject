#ifndef SCENE_SMALL_H
#define SCENE_SMALL_H

#include "SceneVolume.h"
#include <array>

namespace luna
{
	class SceneSmall : public SceneVolume
	{
	protected:
		SceneSmall();
		virtual ~SceneSmall();

		/* The max entity allowed in a small scene */
		const static int MAX_ENTITIES = 25;

		/* find the entity which is not awakened */
		Entity* GetAvailableEntity_() override;

	private:
		/* all entities are here, every scenes have its own total num of entity */
		std::array<Entity, MAX_ENTITIES> m_entities{};
	};
}

#endif
