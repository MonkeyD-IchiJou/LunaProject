#include "CameraControlScript.h"

#include "Input.h"
#include "Entity.h"
#include "CameraComponent.h"
#include "TransformationComponent.h"
#include "Global.h"

#include "DebugLog.h"

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
		auto camera = entity->findComponentT<CameraComponent>(CAMERA_CTYPE);

		if (camera)
		{
			if (input::Mouse.leftclick == true && input::Mouse.numTouchPoints == 1)
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

			int scrolldelta = input::Mouse.scrolldelta;
			float scrollspeed = 15.f;
#if VK_USE_PLATFORM_ANDROID_KHR
			scrollspeed = 1.f;
#endif // VK_USE_ANDROID_PLATFORM


			if (scrolldelta < 0)
			{
				magnitude += global::DeltaTime * 50.f * scrollspeed;
				input::Mouse.scrolldelta = 0;
			}
			else if (scrolldelta > 0)
			{
				magnitude -= global::DeltaTime * 50.f * scrollspeed;
				input::Mouse.scrolldelta = 0;
			}

			entity->transformation->position = camera->target - (glm::normalize(direction) * magnitude);
			//camera->target = position + direction;
			camera->up = up;
		}
	}
}
