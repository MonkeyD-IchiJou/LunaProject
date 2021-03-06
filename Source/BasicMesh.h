#ifndef BASIC_MESH_H
#define BASIC_MESH_H

#include "VulkanBufferObject.h"
#include "Vertex.h"

namespace luna
{
	class BasicMesh : public VulkanBufferObject
	{
	public:
		BasicMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		virtual ~BasicMesh();

		void MapToDeviceMemory(const VkDevice& logicaldevice, const VkDeviceMemory& devicememory) override;

	private:
		/* all the vertice is here */
		std::vector<Vertex> m_vertices;
	};
}

#endif

