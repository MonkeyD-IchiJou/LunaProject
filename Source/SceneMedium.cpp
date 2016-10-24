#include "SceneMedium.h"
#include "DebugLog.h"

namespace luna
{
	SceneMedium::SceneMedium()
	{
	}

	SceneMedium::~SceneMedium()
	{
	}

	Entity * SceneMedium::GetAvailableEntity_()
	{
		for (auto & entity : m_entities)
		{
			// if it is not awakened yet 
			if (!entity.isAwake())
			{
				return &entity;
			}
		}

		// if all is full
		DebugLog::throwEx("All entity full liao, pls allocate more in the start up");

		return nullptr;
	}
}
