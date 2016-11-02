#ifndef CAMERA_COMPONENT_H
#define CAMERA_COMPONENT_H

#include "Component.h"
#include <glm\glm.hpp>

namespace luna
{
	class CameraComponent :
		public Component
	{
	public:
		CameraComponent();
		virtual ~CameraComponent();

		void Update() override;
		void Reset() override;

		glm::mat4 GetView() const { return this->m_view; }
		glm::mat4 GetProjection() const { return this->m_projection; }

	public:
		glm::vec3 target = {};
		glm::vec3 up = {0.f, 1.f, 0.f};
		
		float fovy = 45.f;
		float nearZ = 0.1f;
		float farZ = 10.f;

		bool maincam = false;

	private:
		glm::mat4 m_view = {};
		glm::mat4 m_projection = {};
	};
}

#endif

