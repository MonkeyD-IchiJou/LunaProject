#include "SSBO.h"
#include "DebugLog.h"
#include "Renderer.h"

namespace luna
{
	SSBO::SSBO()
	{
		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();

		// prepare the command buffer
		CommandBufferInit_();
	}

	void SSBO::Update(const std::vector<InstanceData>& ssbo)
	{
		VkDeviceSize prevsize = m_ssboTotalSize;

		// get the total size in bytes
		m_ssboTotalSize = ssbo.size() * sizeof(InstanceData);

		// if the prevsize is not equal to current size.. means something has changed 
		if (prevsize != m_ssboTotalSize)
		{
			// reinit the buffer again because size has changed
			BufferDeInit_();
			BufferInit_();

			// IMPORTANT: need to rewrite the descriptor sets for SSBO before calling vkCmdBindDescriptorSets
		}

		/* begin to record the latest ubo info into the staged device memory */
		void* data = nullptr;
		vkMapMemory(m_logicaldevice, m_staging_mem, 0, m_ssboTotalSize, 0, &data);
		memcpy(data, ssbo.data(), (size_t)m_ssboTotalSize);
		vkUnmapMemory(m_logicaldevice, m_staging_mem);

		/* then copy into the main buffer */
		CopyToDeviceMemory_();
	}

	void SSBO::Update(const std::vector<FontInstanceData>& ssbo)
	{
		VkDeviceSize prevsize = m_ssboTotalSize;

		// get the total size in bytes
		m_ssboTotalSize = ssbo.size() * sizeof(FontInstanceData);

		// if the prevsize is not equal to current size.. means something has changed 
		if (prevsize != m_ssboTotalSize)
		{
			// reinit the buffer again because size has changed
			BufferDeInit_();
			BufferInit_();

			// IMPORTANT: need to rewrite the descriptor sets for SSBO before calling vkCmdBindDescriptorSets
		}

		/* begin to record the latest ubo info into the staged device memory */
		void* data = nullptr;
		vkMapMemory(m_logicaldevice, m_staging_mem, 0, m_staging_buffer.RequirementSizeInDeviceMem, 0, &data);
		memcpy(data, ssbo.data(), (size_t)m_ssboTotalSize);
		vkUnmapMemory(m_logicaldevice, m_staging_mem);

		/* then copy into the main buffer */
		CopyToDeviceMemory_();
	}

	void SSBO::BufferInit_()
	{
		// staging buffer create
		{
			VkBufferCreateInfo buffinfo{};
			buffinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only be used from the graphics queue
			buffinfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // later will transfer into gpu device memory
			buffinfo.size = m_ssboTotalSize; // total sizes in the buffer
			vkCreateBuffer(m_logicaldevice, &buffinfo, nullptr, &m_staging_buffer.buffer);
		}

		// get all the memory requirements info about this buffer
		{
			VkMemoryRequirements memoryrequirement{};
			vkGetBufferMemoryRequirements(m_logicaldevice, m_staging_buffer.buffer, &memoryrequirement);
			m_staging_buffer.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memoryrequirement.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); // find the best heap in the GPU to store at
			m_staging_buffer.BufferTotalSize = m_ssboTotalSize;
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
			buffinfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
			buffinfo.size = m_ssboTotalSize; // total sizes in the buffer
			vkCreateBuffer(m_logicaldevice, &buffinfo, nullptr, &m_main_buffer.buffer);
		}

		// get all the memory requirements info about this buffer
		{
			VkMemoryRequirements memoryrequirement{};
			vkGetBufferMemoryRequirements(m_logicaldevice, m_main_buffer.buffer, &memoryrequirement);

			m_main_buffer.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memoryrequirement.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // find the best heap in the GPU to store at
			m_main_buffer.BufferTotalSize = m_ssboTotalSize;
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

	void SSBO::CommandBufferInit_()
	{
		/* create the command pool first */
		VkCommandPoolCreateInfo commandPool_createinfo{};
		commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		commandPool_createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		commandPool_createinfo.queueFamilyIndex = Renderer::getInstance()->GetQueueFamilyIndices().graphicsFamily;

		DebugLog::EC(vkCreateCommandPool(m_logicaldevice, &commandPool_createinfo, nullptr, &m_commandPool));

		// command buffer creation
		{
			VkCommandBufferAllocateInfo buffer_allocateInfo{};
			buffer_allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			buffer_allocateInfo.commandPool = m_commandPool;
			buffer_allocateInfo.commandBufferCount = 1;
			buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

			DebugLog::EC(vkAllocateCommandBuffers(m_logicaldevice, &buffer_allocateInfo, &m_commandbuffer));
		}
	}

	void SSBO::CopyToDeviceMemory_()
	{
		// immediately start recording this command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		vkBeginCommandBuffer(m_commandbuffer, &beginInfo);

		// start the copy cmd
		// copy the damn buffer
		VkBufferCopy copyregion{};
		copyregion.srcOffset = 0;
		copyregion.dstOffset = 0;
		copyregion.size = m_ssboTotalSize;
		vkCmdCopyBuffer(m_commandbuffer, m_staging_buffer.buffer, m_main_buffer.buffer, 1, &copyregion);

		vkEndCommandBuffer(m_commandbuffer);

		// then submit this to the graphics queue to execute it
		// now execute the command buffer
		VkSubmitInfo submitinfo{};
		submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &m_commandbuffer;

		vkQueueSubmit(Renderer::getInstance()->GetGraphicQueue(), 1, &submitinfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(Renderer::getInstance()->GetGraphicQueue());
	}

	void SSBO::BufferDeInit_()
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

	SSBO::~SSBO()
	{
		if (m_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_logicaldevice, m_commandPool, nullptr);
		}

		BufferDeInit_();
	}
}
