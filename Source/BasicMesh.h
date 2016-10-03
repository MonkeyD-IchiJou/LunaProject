#ifndef BASIC_MESH_H
#define BASIC_MESH_H

#include "VulkanBufferObject.h"
#include <vector>
#include "Vertex.h"

namespace luna
{
	class BasicMesh : public VulkanBufferObject
	{
	public:
		BasicMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices);
		virtual ~BasicMesh();

		void Draw(const VkCommandBuffer& commandbuffer);
		void DrawInstanced(const VkCommandBuffer& commandbuffer, const uint32_t& instancecount);
		void MapToDeviceMemory(const VkDevice& logicaldevice, const VkDeviceMemory& devicememory);

		inline auto getVertexTotalSize() const { return m_vertexTotalSize; }
		inline auto getIndexTotalSize() const { return m_indexTotalSize; }
		inline auto getVertexOffset() const { return m_vertexOffset; }
		inline auto getIndexOffset() const { return m_indexOffset; }
		inline void setMainBuffer(VkBuffer buffer) { this->m_main_buffer = buffer; }

	protected:
		/* all the vertice is here */
		std::vector<Vertex> m_vertices;

		/* all the indice is here */
		std::vector<uint32_t> m_indices;

		/* total vertex size in bytes */
		VkDeviceSize m_vertexTotalSize = 0;

		/* total index size in bytes */
		VkDeviceSize m_indexTotalSize = 0;

		/* the beginning offset (bytes) of the vertex data to be mapped of*/
		VkDeviceSize m_vertexOffset = 0;

		/* the beginning offset (bytes) of the index data to be mapped of*/
		VkDeviceSize m_indexOffset = 0;

		/* the main buffer to communicate with when drawing */
		VkBuffer m_main_buffer = VK_NULL_HANDLE;
	};
}

#endif

