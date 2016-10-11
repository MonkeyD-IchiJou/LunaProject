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
		SSBO(const VkDeviceSize& reservesize);
		~SSBO();

		// update the ubodata into device memory
		template<typename T>
		void Update(const std::vector<T>& ssbo)
		{
			// get the total size in bytes
			m_currentSize = ssbo.size() * sizeof(T);

			// if the current size is more than the total reserve size,
			// resize it
			if (m_currentSize >= m_ssboTotalSize)
			{
				// reinit the buffer again because size has changed
				m_ssboTotalSize = m_ssboTotalSize * 2; // double amount the size

				BufferDeInit_();
				BufferInit_();

				// IMPORTANT: need to rewrite the descriptor sets for SSBO before calling vkCmdBindDescriptorSets
			}

			/* begin to record the latest ubo info into the staged device memory */
			void* data = nullptr;
			vkMapMemory(m_logicaldevice, m_staging_mem, 0, static_cast<size_t>(m_currentSize), 0, &data);
			memcpy(data, ssbo.data(), static_cast<size_t>(m_currentSize));
			vkUnmapMemory(m_logicaldevice, m_staging_mem);

			/* then copy into the main buffer */
			CopyToDeviceMemory_();
		}

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
		VkDeviceSize m_currentSize = 0;

		/* the main buffer to communicate with when updating */
		VulkanBufferData m_main_buffer{};
		VkDeviceMemory m_main_memory = VK_NULL_HANDLE;

		/* staging buffer for updating ubodata */
		VulkanBufferData m_staging_buffer{};
		VkDeviceMemory m_staging_mem = VK_NULL_HANDLE;

		/* command buffer for copying buffer between staging and main buffer */
		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_commandbuffer = VK_NULL_HANDLE;

		VkCommandBufferBeginInfo m_beginInfo{};
		VkBufferCopy m_copyregion{};
		VkSubmitInfo m_submitinfo{};

		// logical device handle
		VkDevice m_logicaldevice = VK_NULL_HANDLE;
		VkQueue m_queue = VK_NULL_HANDLE;
	};
}

#endif

