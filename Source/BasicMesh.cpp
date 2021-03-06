#include "BasicMesh.h"
#include "DebugLog.h"

namespace luna
{

	BasicMesh::BasicMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices)
	{
		m_vertices = vertices;
		m_indices = indices;

		// store the total size for it
		m_vertexTotalSize = sizeof(m_vertices[0]) * m_vertices.size();
		m_indexTotalSize = sizeof(m_indices[0]) * m_indices.size();

		// give it the offset 
		m_vertexOffset = CurrentBufferTotalSize; 
		CurrentBufferTotalSize += m_vertexTotalSize; /* increase the current buffer size for the next offset */

		m_indexOffset = CurrentBufferTotalSize; 
		CurrentBufferTotalSize += m_indexTotalSize; /* increase the current buffer size for the next offset */
	}

	BasicMesh::~BasicMesh()
	{
	}

	void BasicMesh::MapToDeviceMemory(const VkDevice & logicaldevice, const VkDeviceMemory & devicememory)
	{
		if (devicememory == VK_NULL_HANDLE)
		{
			DebugLog::throwEx("device memory is undefined");
			return;
		}

		/* begin to record the vertices info into the devicememory */
		{
			void* data = nullptr;
			vkMapMemory(logicaldevice, devicememory, m_vertexOffset, m_vertexTotalSize, 0, &data);
			memcpy(data, m_vertices.data(), (size_t) m_vertexTotalSize);
			vkUnmapMemory(logicaldevice, devicememory);
		}

		/* begin to record the indices info into the devicememory */
		{
			void* data = nullptr;
			vkMapMemory(logicaldevice, devicememory, m_indexOffset, m_indexTotalSize, 0, &data);
			memcpy(data, m_indices.data(), (size_t) m_indexTotalSize);
			vkUnmapMemory(logicaldevice, devicememory);
		}
	}
}
