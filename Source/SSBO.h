#ifndef BASIC_SSBO_H
#define BASIC_SSBO_H

#include "StorageData.h"
#include "VulkanBufferData.h"
#include <vector>

namespace luna
{
	class SSBO
	{
	public:
		SSBO();
		~SSBO();

		// update the ubodata into device memory
		void Update(const std::vector<InstanceData>& ssbo);
		
		inline auto getSSBOTotalSize() const { return m_ssboTotalSize; }
		inline auto getMainBuffer() const { return m_main_buffer; }

	private:
		void BufferInit_();
		void BufferDeInit_();
		void CommandBufferInit_();
		void CopyToDeviceMemory_();

	protected:
		/* total ubo size in bytes */
		VkDeviceSize m_ssboTotalSize = 0;

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

