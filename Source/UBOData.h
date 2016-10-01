#ifndef UBO_DATA_H
#define UBO_DATA_H

#include <glm\glm.hpp>

namespace luna
{
	struct UBOData
	{
		glm::mat4 model, view, proj;
	};
}

#endif