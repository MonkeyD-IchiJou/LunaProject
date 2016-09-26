#ifndef BASIC_UBO_H
#define BASIC_UBO_H

#include "VulkanBufferObject.h"
#include "UBOData.h"
#include "VulkanBufferData.h"

namespace luna
{
	class BasicUBO :
		public VulkanBufferObject
	{
	public:
		BasicUBO();
		virtual ~BasicUBO();

		void Update(const UBOData& ubodata);
		void MapToDeviceMemory(const VkDeviceMemory& devicememory);

		inline auto getUboTotalSize() const { return m_uboTotalSize; }
		inline auto getUboOffset() const { return m_uboOffset; }
		inline auto getMainBuffer() const { return m_MainBuffer; }
		inline void setMainBuffer(VkBuffer buffer) { this->m_MainBuffer = buffer; }

	private:
		void StagingBufferInit_();
		void CommandBufferInit_();
		void CopyToDeviceMemory_();

	protected:
		/* the actual data */
		UBOData m_ubodata;

		/* total ubo size in bytes */
		VkDeviceSize m_uboTotalSize = 0;

		/* the beginning offset (bytes) of the ubo data to be mapped of*/
		VkDeviceSize m_uboOffset = 0;

		/* the main buffer to communicate with when updating */
		VkBuffer m_MainBuffer = VK_NULL_HANDLE;

		/* staging buffer for updating ubodata */
		VulkanBufferData staging_buff{};
		VkDeviceMemory staging_mem = VK_NULL_HANDLE;

		VkCommandPool m_commandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_commandbuffer = VK_NULL_HANDLE;

		// logical device handle
		VkDevice m_logicaldevice = VK_NULL_HANDLE;
	};
}

#endif

