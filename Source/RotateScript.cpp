#include "RotateScript.h"
#include "Entity.h"
#include "TransformationComponent.h"
#include <glm\glm.hpp>

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
		auto t = entity->transformation;
		t->rotation.x = 0.f; t->rotation.y = 1.f; t->rotation.x = 0.f;
		t->rotation.w += 0.1f;
	}
}
