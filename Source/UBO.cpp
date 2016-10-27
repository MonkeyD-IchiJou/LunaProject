#include "UBO.h"
#include "DebugLog.h"
#include "Renderer.h"

namespace luna
{
	UBO::UBO()
	{
		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();

		m_uboTotalSize = sizeof(UBOData);

		BufferInit_();
	}

	void UBO::Update(const UBOData & ubodata)
	{
		// update the new ubo data 
		/* begin to record the latest ubo info into the staged device memory */
		void* data = nullptr;
		vkMapMemory(m_logicaldevice, m_staging_mem, 0, m_uboTotalSize, 0, &data);
		memcpy(data, &ubodata, (size_t)m_uboTotalSize);
		vkUnmapMemory(m_logicaldevice, m_staging_mem);
	}

	void UBO::Record(const VkCommandBuffer cmdbuff)
	{
		// start the copy cmd
		// copy the damn buffer
		VkBufferCopy copyregion{};
		copyregion.srcOffset = 0;
		copyregion.dstOffset = 0;
		copyregion.size = m_uboTotalSize;

		vkCmdCopyBuffer(cmdbuff, m_staging_buffer.buffer, m_main_buffer.buffer, 1, &copyregion);
	}

	void UBO::BufferInit_()
	{
		// staging buffer create
		{
			VkBufferCreateInfo buffinfo{};
			buffinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only be used from the graphics queue
			buffinfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // later will transfer into gpu device memory
			buffinfo.size = m_uboTotalSize; // total sizes in the buffer
			vkCreateBuffer(m_logicaldevice, &buffinfo, nullptr, &m_staging_buffer.buffer);
		}

		// get all the memory requirements info about this buffer
		{
			VkMemoryRequirements memoryrequirement{};
			vkGetBufferMemoryRequirements(m_logicaldevice, m_staging_buffer.buffer, &memoryrequirement);
			m_staging_buffer.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memoryrequirement.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); // find the best heap in the GPU to store at
			m_staging_buffer.BufferTotalSize = m_uboTotalSize;
			m_staging_buffer.RequirementSizeInDeviceMem = memoryrequirement.size;
			m_staging_buffer.RequirementAlignmentInDeviceMem = memoryrequirement.alignment;
		}

		/* After created the buffer, time to allocate the requirement memory for it */
		// allocate a big devicememory to store these buffers
		{
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = m_staging_buffer.RequirementSizeInDeviceMem;
			allocInfo.memoryTypeIndex = m_staging_buffer.MemoryTypeIndex;

			DebugLog::EC(vkAllocateMemory(m_logicaldevice, &allocInfo, nullptr, &m_staging_mem));

			// bind the buffer at the beginning of 0 offset in the Device Memory
			vkBindBufferMemory(m_logicaldevice, m_staging_buffer.buffer, m_staging_mem, 0);
		}

		/* main ubo buffers below */

		{
			VkBufferCreateInfo buffinfo{};
			buffinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only be used from the graphics queue
			buffinfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			buffinfo.size = m_uboTotalSize; // total sizes in the buffer
			vkCreateBuffer(m_logicaldevice, &buffinfo, nullptr, &m_main_buffer.buffer);
		}

		// get all the memory requirements info about this buffer
		{
			VkMemoryRequirements memoryrequirement{};
			vkGetBufferMemoryRequirements(m_logicaldevice, m_main_buffer.buffer, &memoryrequirement);

			m_main_buffer.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memoryrequirement.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // find the best heap in the GPU to store at
			m_main_buffer.BufferTotalSize = m_uboTotalSize;
			m_main_buffer.RequirementSizeInDeviceMem = memoryrequirement.size;
			m_main_buffer.RequirementAlignmentInDeviceMem = memoryrequirement.alignment;
		}

		/* After created the buffer, time to allocate the requirement memory for it */
		// allocate a big devicememory to store these buffers
		{
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = m_main_buffer.RequirementSizeInDeviceMem;
			allocInfo.memoryTypeIndex = m_main_buffer.MemoryTypeIndex;

			DebugLog::EC(vkAllocateMemory(m_logicaldevice, &allocInfo, nullptr, &m_main_memory));

			// Buffers binding to the memory
			// bind the buffer at the beginning of 0 offset in the Device Memory
			vkBindBufferMemory(m_logicaldevice, m_main_buffer.buffer, m_main_memory, 0);
		}
	}

	UBO::~UBO()
	{
		if (m_staging_buffer.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_logicaldevice, m_staging_buffer.buffer, nullptr);
			m_staging_buffer.buffer = VK_NULL_HANDLE;
		}

		if (m_staging_mem != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_logicaldevice, m_staging_mem, nullptr);
			m_staging_mem = VK_NULL_HANDLE;
		}

		if (m_main_buffer.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_logicaldevice, m_main_buffer.buffer, nullptr);
			m_main_buffer.buffer = VK_NULL_HANDLE;
		}

		if (m_main_memory != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_logicaldevice, m_main_memory, nullptr);
			m_main_memory = VK_NULL_HANDLE;
		}
	}
}
