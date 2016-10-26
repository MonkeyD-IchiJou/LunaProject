#ifndef BASICMESH_COMPONENT_H
#define BASICMESH_COMPONENT_H

#include "Component.h"
#include "enum_c.h"
#include <glm\glm.hpp>

namespace luna
{
	struct BasicMeshMaterial
	{
		MESH_TEX textureID = MESH_TEX::MAX_MESH_TEX;
		glm::vec4 color = {};
	};

	class BasicMeshComponent :
		public Component
	{
	public:
		BasicMeshComponent();
		virtual ~BasicMeshComponent();

		void Update() override;
		void Reset() override;

		eMODELS meshID = eMODELS::MAX_MODELS;
		BasicMeshMaterial material = {};
	};
}

#endif // !BASICMESH_COMPONENT_H

