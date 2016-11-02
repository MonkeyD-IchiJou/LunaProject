#ifndef BASIC_UBO_H
#define BASIC_UBO_H

#include "FramePacket.h"
#include "VulkanBufferData.h"

namespace luna
{
	class UBO
	{
	public:
		UBO();
		~UBO();

		// update the ubodata into device memory
		void Update(const UBOData& ubodata);
		
		inline VkDeviceSize getUboTotalSize() const { return m_uboTotalSize; }
		inline VulkanBufferData getMainBuffer() const { return m_main_buffer; }

		void Record(const VkCommandBuffer cmdbuff);

	private:
		void BufferInit_();

	protected:
		/* total ubo size in bytes */
		VkDeviceSize m_uboTotalSize = 0;

		/* the main buffer to communicate with when updating */
		VulkanBufferData m_main_buffer{};
		VkDeviceMemory m_main_memory = VK_NULL_HANDLE;

		/* staging buffer for updating ubodata */
		VulkanBufferData m_staging_buffer{};
		VkDeviceMemory m_staging_mem = VK_NULL_HANDLE;

		// logical device handle
		VkDevice m_logicaldevice = VK_NULL_HANDLE;
	};
}

#endif

