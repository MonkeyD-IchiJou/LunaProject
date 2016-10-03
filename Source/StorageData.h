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
	};
}

#endif