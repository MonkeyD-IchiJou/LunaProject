#include "PointlightRandomScript.h"

#include "Entity.h"
#include "TransformationComponent.h"
#include "Global.h"
#include "PointLightComponent.h"

#include <cstdlib>
#include <ctime>

namespace luna
{

	PointlightRandomScript::PointlightRandomScript()
	{
		static bool seedonce = false;
		if (!seedonce)
		{
			srand (static_cast <unsigned> (time(0)));
			seedonce = true;
		}
	}

	PointlightRandomScript::~PointlightRandomScript()
	{
	}

	void PointlightRandomScript::Update(Entity * entity)
	{
		auto pointlight = entity->findComponentT<PointLightComponent>();
		auto transformation = entity->transformation;
		
		if (!firstInit)
		{
			float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
			float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

			pointlight->color = glm::vec3(r, g, b);
			
			float LO = 0.5f;
			float HI = 2.25f;
			speed.x = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
			speed.y = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));
			speed.z = LO + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(HI-LO)));

			firstInit = true;
		}

		transformation->position =
			center + 
			glm::vec3(cos(angle.x), sin(angle.y), cos(angle.z)) * 
			radius;

		angle += speed * glm::vec3(global::DeltaTime);

		if (angle.x >= 360.f)
			angle.x -= 360.f;
		if (angle.y >= 360.f)
			angle.y -= 360.f;
		if (angle.z >= 360.f)
			angle.z -= 360.f;
	}

}
