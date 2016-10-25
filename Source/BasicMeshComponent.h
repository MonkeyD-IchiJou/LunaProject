#ifndef BASICMESH_COMPONENT_H
#define BASICMESH_COMPONENT_H

#include "Component.h"
#include "enum_c.h"
#include <glm\glm.hpp>

namespace luna
{
	struct BasicMeshMaterial
	{
		eTEXTURES textureID = eTEXTURES::MAXTEX_NAME_TYPE_FORMAT;
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

	private:
		// only used for comparison in finding
		friend bool operator== (const BasicMeshComponent& n1, const BasicMeshComponent& n2);
	};
}

#endif // !BASICMESH_COMPONENT_H

