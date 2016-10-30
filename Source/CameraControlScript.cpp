#include "CameraControlScript.h"

#include "Input.h"
#include "Entity.h"
#include "CameraComponent.h"
#include "TransformationComponent.h"
#include "Global.h"

namespace luna
{
	CameraControlScript::CameraControlScript()
	{
		mouseSpeed = { -0.005f, 0.005f };
		speed = 3.0f;
	}

	CameraControlScript::~CameraControlScript()
	{
	}

	void CameraControlScript::Update(Entity * entity)
	{
		auto camera = entity->findComponentT<CameraComponent>();

		if (camera)
		{
			if (input::Mouse.leftclick == true)
			{
				if (firstclick == true)
				{
					currentmousepos = glm::vec2(input::Mouse.posx, input::Mouse.posy);
					prevmousepos = currentmousepos;
					firstclick = false;
				}
				else
				{
					prevmousepos = currentmousepos;
					currentmousepos = glm::vec2(input::Mouse.posx, input::Mouse.posy);
				}

				// Compute new orientation
				Angle += mouseSpeed * (prevmousepos - currentmousepos);
			}
			else
			{
				firstclick = true;
			}

			// Direction : Spherical coordinates to Cartesian coordinates conversion
			const auto cosvertangle = cos(Angle.y);
			glm::vec3 direction(
				cosvertangle * sin(Angle.x),
				sin(Angle.y),
				cosvertangle * cos(Angle.x)
			);

			// Up vector
			glm::vec3 up{};
			auto rh = Angle.x - (3.14f / 2.0f);

			// Right vector
			glm::vec3 right = glm::vec3(
				sin(rh),
				0,
				cos(rh)
			);

			up = glm::cross(right, direction);

			auto& position = entity->transformation->position;
			if (input::Keys['W'].pressed == true)
			{
				position += direction * (speed * global::DeltaTime);
			}
			if (input::Keys['A'].pressed == true)
			{
				position += right * (speed * global::DeltaTime);
			}
			if (input::Keys['S'].pressed == true)
			{
				position -= direction * (speed * global::DeltaTime);
			}
			if (input::Keys['D'].pressed == true)
			{
				position -= right * (speed * global::DeltaTime);
			}

			camera->target = position + direction;
			camera->up = up;
		}
	}
}
