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
		//t->eulerangles.x += 10.0f * global::DeltaTime;
		t->eulerangles.y += 700.0f * global::DeltaTime;
		//t->eulerangles.z += 10.0f * global::DeltaTime;

		/*if (!change)
		{
			timer += global::DeltaTime;
			t->position.y += 30.f * global::DeltaTime;

			if (timer > 0.5f)
			{
				change = true;
				timer = 0.f;
			}
		}
		else
		{
			timer += global::DeltaTime;
			t->position.y -= 30.f * global::DeltaTime;

			if (timer > 0.5f)
			{
				change = false;
				timer = 0.f;
			}
		}*/
	
		if (t->eulerangles.x > 360.f)
			t->eulerangles.x -= 360.f;
		if (t->eulerangles.y > 360.f)
			t->eulerangles.y -= 360.f;
		if (t->eulerangles.z > 360.f)
			t->eulerangles.z -= 360.f;
		
	}
}
