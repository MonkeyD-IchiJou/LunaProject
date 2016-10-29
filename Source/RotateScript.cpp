#include "RotateScript.h"
#include "Entity.h"
#include "TransformationComponent.h"
#include <glm\glm.hpp>

#include "Input.h"
#include "enum_c.h"

#include "DebugLog.h"

namespace luna
{
	RotateScript::RotateScript()
	{
	}

	RotateScript::~RotateScript()
	{
	}

	void RotateScript::Update(Entity * entity)
	{
		//DebugLog::printL(input::Keys['A'].pressed);

		if (input::Mouse.leftclick == true)
		{
			auto t = entity->transformation;
			t->rotation.x = 0.f; t->rotation.y = 1.f; t->rotation.x = 0.f;
			t->rotation.w += 0.1f;
		}

		if (input::Keys['\r'].pressed == true)
		{
			auto t = entity->transformation;
			t->position.x += 0.001f;
		}
	}
}
