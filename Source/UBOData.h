#ifndef UBO_DATA_H
#define UBO_DATA_H

#include "vk_glm.h"

namespace luna
{
	struct UBOData
	{
		glm::mat4 model, view, proj;
	};
}

#endif