#include "BasicUBO.h"
#include "DebugLog.h"
#include "Renderer.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

namespace luna
{

	BasicUBO::BasicUBO()
	{
		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();

		m_ubodata.model = glm::translate(glm::mat4(), glm::vec3(0.f, 0.f, 0.f));
		m_ubodata.view = glm::lookAt(glm::vec3(0.f, 1.f, 8.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		m_ubodata.proj = glm::perspective(glm::radians(45.f), 1080.f / 720.f, 0.1f, 10.0f); // take note .. hardcoded aspects

		m_uboTotalSize = sizeof(UBOData);

		// give it the offset 
		m_uboOffset = CurrentBufferTotalSize; 
		CurrentBufferTotalSize += m_uboTotalSize; /* increase the current buffer size for the next offset */

		StagingBufferInit_();
		CommandBufferInit_();
	}

	void BasicUBO::Update(const UBOData & ubodata)
	{
		// update the new ubo data 
		m_ubodata = ubodata;

		/* begin to record the latest ubo info into the staged device memory */
		MapToDeviceMemory(staging_mem);

		/* then copy into the main buffer */
		CopyToDeviceMemory_();
	}

	void BasicUBO::MapToDeviceMemory(const VkDeviceMemory & devicememory)
	{
		/* begin to record the ubo info into the devicememory */
		{
			void* data = nullptr;
			vkMapMemory(m_logicaldevice, devicememory, m_uboOffset, m_uboTotalSize, 0, &data);
			memcpy(data, &m_ubodata, (size_t) m_uboTotalSize);
			vkUnmapMemory(m_logicaldevice, devicememory);
		}
	}

	void BasicUBO::StagingBufferInit_()
	{
		/* After all the meshes have inited, create a big buffer to contain all of them */
		// create my big buffer
		{
			VkBufferCreateInfo buffinfo{};
			buffinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only be used from the graphics queue
			buffinfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // later will transfer into gpu device memory
			buffinfo.size = m_uboTotalSize; // total sizes in the buffer
			vkCreateBuffer(m_logicaldevice, &buffinfo, nullptr, &staging_buff.buffer);
		}

		// get all the memory requirements info about this buffer
		{
			VkMemoryRequirements memoryrequirement{};
			vkGetBufferMemoryRequirements(m_logicaldevice, staging_buff.buffer, &memoryrequirement);
			staging_buff.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memoryrequirement.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); // find the best heap in the GPU to store at
			staging_buff.BufferTotalSize = m_uboTotalSize;
			staging_buff.RequirementSizeInDeviceMem = memoryrequirement.size;
			staging_buff.RequirementAlignmentInDeviceMem = memoryrequirement.alignment;
		}

		/* After created the buffer, time to allocate the requirement memory for it */
		// allocate a big devicememory to store these buffers
		{
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = staging_buff.RequirementSizeInDeviceMem;
			allocInfo.memoryTypeIndex = staging_buff.MemoryTypeIndex;

			DebugLog::EC(vkAllocateMemory(m_logicaldevice, &allocInfo, nullptr, &staging_mem));

			// bind the buffer at the beginning of 0 offset in the Device Memory
			vkBindBufferMemory(m_logicaldevice, staging_buff.buffer, staging_mem, 0);
		}
	}

	void BasicUBO::CommandBufferInit_()
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

	void BasicUBO::CopyToDeviceMemory_()
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
		copyregion.size = m_uboTotalSize;
		vkCmdCopyBuffer(m_commandbuffer, staging_buff.buffer, m_MainBuffer, 1, &copyregion);

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

	BasicUBO::~BasicUBO()
	{
		if (m_commandPool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(m_logicaldevice, m_commandPool, nullptr);
		}

		if (staging_buff.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_logicaldevice, staging_buff.buffer, nullptr);
			staging_buff.buffer = VK_NULL_HANDLE;
		}

		if (staging_mem != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_logicaldevice, staging_mem, nullptr);
			staging_mem = VK_NULL_HANDLE;
		}
	}
}
