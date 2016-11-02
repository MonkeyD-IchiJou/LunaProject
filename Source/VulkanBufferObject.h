#ifndef VULKAN_BUFFER_OBJ_H
#define VULKAN_BUFFER_OBJ_H

#include "platform.h"
#include <vector>

namespace luna
{
	class VulkanBufferObject
	{
	public:
		VulkanBufferObject();
		virtual ~VulkanBufferObject();

		virtual void Bind(const VkCommandBuffer& commandbuffer);
		virtual void Draw(const VkCommandBuffer& commandbuffer);
		virtual void DrawInstanced(const VkCommandBuffer& commandbuffer, const uint32_t& instancecount);
		virtual void MapToDeviceMemory(const VkDevice& logicaldevice, const VkDeviceMemory& devicememory) = 0;

		inline VkDeviceSize getVertexTotalSize() const { return m_vertexTotalSize; }
		inline VkDeviceSize getIndexTotalSize() const { return m_indexTotalSize; }
		inline VkDeviceSize getVertexOffset() const { return m_vertexOffset; }
		inline VkDeviceSize getIndexOffset() const { return m_indexOffset; }
		inline void setMainBuffer(VkBuffer buffer) { this->m_main_buffer = buffer; }

		/* current total size in this buffer */
		static VkDeviceSize CurrentBufferTotalSize;

	protected:
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
