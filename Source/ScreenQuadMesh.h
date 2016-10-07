#ifndef SCREEN_QUAD_MESH_H
#define SCREEN_QUAD_MESH_H

#include "VulkanBufferObject.h"
#include "Vertex.h"

namespace luna
{
	class ScreenQuadMesh :
		public VulkanBufferObject
	{
	public:
		ScreenQuadMesh();
		virtual ~ScreenQuadMesh();

		void MapToDeviceMemory(const VkDevice& logicaldevice, const VkDeviceMemory& devicememory) override;

	private:
		/* all the vertice is here */
		std::vector<ScreenQuadVertex> m_vertices;
	};
}

#endif

