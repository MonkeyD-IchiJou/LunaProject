#ifndef BASIC_UBO_H
#define BASIC_UBO_H

#include "StorageData.h"
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
		
		inline auto getUboTotalSize() const { return m_uboTotalSize; }
		inline auto getUboOffset() const { return m_uboOffset; }
		inline auto getMainBuffer() const { return m_main_buffer; }

	private:
		void BufferInit_();
		void CommandBufferInit_();
		void CopyToDeviceMemory_();

	protected:
		/* total ubo size in bytes */
		VkDeviceSize m_uboTotalSize = 0;

		/* the beginning offset (bytes) of the ubo data to be mapped of*/
		VkDeviceSize m_uboOffset = 0;

		/* the main buffer to communicate with when updating */
		VulkanBufferData m_main_buffer{};
		VkDeviceMemory m_main_memory = VK_NULL_HANDLE;

		/* staging buffer for updating ubodata */
		VulkanBufferData m_staging_buffer{};
		VkDeviceMemory m_staging_mem = VK_NULL_HANDLE;

		/* command buffer for copying buffer between staging and main buffer */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_commandbuffer = VK_NULL_HANDLE;

		// logical device handle
		VkDevice m_logicaldevice = VK_NULL_HANDLE;
	};
}

#endif

