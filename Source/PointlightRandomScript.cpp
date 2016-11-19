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
			transformation->position = glm::vec3(r, -5.f, b);
			dir = glm::vec3(r, 0.f, b);
			firstInit = true;
		}

		transformation->position += dir * glm::vec3(global::DeltaTime, global::DeltaTime, global::DeltaTime);
	}

}
