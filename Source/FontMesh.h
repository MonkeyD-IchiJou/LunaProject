#ifndef FONT_MESH_H
#define FONT_MESH_H

#include "VulkanBufferObject.h"
#include "Vertex.h"

namespace luna
{
	class FontMesh : public VulkanBufferObject
	{
	public:
		FontMesh();
		virtual ~FontMesh();

		void MapToDeviceMemory(const VkDevice& logicaldevice, const VkDeviceMemory& devicememory) override;

	private:
		/* all the vertices is here */
		std::vector<FontVertex> m_vertices;
	};
}

#endif

