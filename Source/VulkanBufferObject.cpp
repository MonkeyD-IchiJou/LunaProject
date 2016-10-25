#include "VulkanBufferObject.h"
#include "DebugLog.h"

namespace luna
{
	VkDeviceSize VulkanBufferObject::CurrentBufferTotalSize = 0;

	VulkanBufferObject::VulkanBufferObject()
	{
	}

	VulkanBufferObject::~VulkanBufferObject()
	{
	}

	void VulkanBufferObject::Bind(const VkCommandBuffer & commandbuffer)
	{
		// vertices data binding at 0
		vkCmdBindVertexBuffers(commandbuffer, 0, 1, &m_main_buffer, &m_vertexOffset);
		vkCmdBindIndexBuffer(commandbuffer, m_main_buffer, m_indexOffset, VK_INDEX_TYPE_UINT32);
	}

	void VulkanBufferObject::Draw(const VkCommandBuffer & commandbuffer)
	{
		Bind(commandbuffer);
		vkCmdDrawIndexed(commandbuffer, (uint32_t) m_indices.size(), 1, 0, 0, 0);
	}

	void VulkanBufferObject::DrawInstanced(const VkCommandBuffer & commandbuffer, const uint32_t& instancecount)
	{
		Bind(commandbuffer);
		vkCmdDrawIndexed(commandbuffer, (uint32_t) m_indices.size(), instancecount, 0, 0, 0);
	}
}
