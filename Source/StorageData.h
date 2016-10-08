#ifndef STORAGE_DATA_H
#define STORAGE_DATA_H

#include <glm\glm.hpp>

namespace luna
{
	struct UBOData
	{
		glm::mat4 view, proj;
	};

	struct InstanceData
	{
		glm::mat4 model;
		glm::mat4 transpose_inverse_model;
	};

	struct FontInstanceData
	{
		glm::mat4 transformation;
		glm::mat4 fontMaterials;
		glm::vec2 uv[4];
	};
}

#endif