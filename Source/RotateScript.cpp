#include "RotateScript.h"
#include "Entity.h"
#include "TransformationComponent.h"
#include <glm\glm.hpp>

#include "Input.h"
#include "enum_c.h"
#include "Global.h"

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
		auto t = entity->transformation;
		t->eulerangles.x += 50.0f * global::DeltaTime;
		t->eulerangles.y += 50.0f * global::DeltaTime;
		t->eulerangles.z += 50.0f * global::DeltaTime;
	
		if (t->eulerangles.x > 360.f)
			t->eulerangles.x -= 360.f;
		if (t->eulerangles.y > 360.f)
			t->eulerangles.y -= 360.f;
		if (t->eulerangles.z > 360.f)
			t->eulerangles.z -= 360.f;
		
	}
}
