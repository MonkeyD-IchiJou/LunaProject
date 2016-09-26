#ifndef VERTEX_H
#define VERTEX_H

#include "Platform.h"
#include <array>

#include "vk_glm.h"

namespace luna
{ 
	struct Vertex
	{
		glm::vec3 pos;
		glm::vec3 normal;
		glm::vec2 texCoord;

		static inline VkVertexInputBindingDescription getBindingDescription()
		{
			VkVertexInputBindingDescription bindingDescription{};
			bindingDescription.binding = 0;
			bindingDescription.stride = sizeof(Vertex);
			bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX; // instancing -> VK_VERTEX_INPUT_RATE_INSTANCE

			return bindingDescription;
		}

		static inline std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions()
		{
			std::array<VkVertexInputAttributeDescription, 3> attributeDescription{};

			attributeDescription[0].binding = 0;
			attributeDescription[0].location = 0;
			attributeDescription[0].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription[0].offset = offsetof(Vertex, pos);

			attributeDescription[1].binding	= 0;
			attributeDescription[1].location = 1;
			attributeDescription[1].format = VK_FORMAT_R32G32B32_SFLOAT;
			attributeDescription[1].offset = offsetof(Vertex, normal);

			attributeDescription[2].binding	= 0;
			attributeDescription[2].location = 2;
			attributeDescription[2].format = VK_FORMAT_R32G32_SFLOAT;
			attributeDescription[2].offset = offsetof(Vertex, texCoord);

			return attributeDescription;
		}

		bool operator==(const Vertex& other) const
		{
			return pos == other.pos && normal == other.normal && texCoord == other.texCoord;
		}
	};
}

#endif

