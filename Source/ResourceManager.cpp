#include "ResourceManager.h"
#include "Model.h"
#include "BasicMesh.h"
#include "DebugLog.h"
#include "Renderer.h"
#include "BasicUBO.h"

namespace luna
{
	std::once_flag ResourceManager::m_sflag{};
	ResourceManager* ResourceManager::m_instance = nullptr;

	ResourceManager::ResourceManager()
	{
		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();

		// init all the textures and meshes
		Init_();
	}

	ResourceManager::~ResourceManager()
	{
	}

	void ResourceManager::Init_()
	{
		// ubo first
		UBO = new BasicUBO();

		// init all the mesh
		Models.resize(MAX_MODELS);
		Models[eMODELS::QUAD_MODEL] = new Model(ePRIMITIVE_MESH::PRIMITIVE_QUAD);
		Models[eMODELS::BOXES_MODEL] = new Model("./../Assets/Models/boxes.lrl");
		Models[eMODELS::BUNNY_MODEL] = new Model("./../Assets/Models/bunny.lrl");
		Models[eMODELS::TYRA_MODEL] = new Model("./../Assets/Models/tyra.lrl");
		

		// after retrieved all the models datas 
		LoadToDevice_();
	}

	void ResourceManager::LoadToDevice_()
	{
		/* buffer creation based on all the resources sizes, then allocate the device memory to it */
		StagingBufferInit_();

		/* map all the data to the staged device memory */
		UBO->MapToDeviceMemory(m_devicememory_staged);
		for (auto &model : Models)
		{
			model->MapToDeviceMemory(m_logicaldevice, m_devicememory_staged);
		}

		/* copy the buffer datas to the real buffer */
		DeviceBufferInit_();
		CopyBufferToDevice_();

		/* lastly tell the mesh which buffer to talk to when rendering */
		UBO->setMainBuffer(m_mainBuffer.buffer);
		for (auto &model : Models)
		{
			model->setMainBuffer(m_mainBuffer.buffer);
		}
	}

	void ResourceManager::StagingBufferInit_()
	{
		/* After all the meshes have inited, create a big buffer to contain all of them */
		// create my big buffer
		{
			VkBufferCreateInfo buffinfo{};
			buffinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only be used from the graphics queue
			buffinfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT; // later will transfer into gpu device memory
			buffinfo.size = VulkanBufferObject::CurrentBufferTotalSize; // total sizes in the buffer
			vkCreateBuffer(m_logicaldevice, &buffinfo, nullptr, &m_stagingBuffer.buffer);
		}

		// get all the memory requirements info about this buffer
		{
			VkMemoryRequirements memoryrequirement{};
			vkGetBufferMemoryRequirements(m_logicaldevice, m_stagingBuffer.buffer, &memoryrequirement);
			m_stagingBuffer.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memoryrequirement.memoryTypeBits,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT); // find the best heap in the GPU to store at
			m_stagingBuffer.BufferTotalSize = VulkanBufferObject::CurrentBufferTotalSize;
			m_stagingBuffer.RequirementSizeInDeviceMem = memoryrequirement.size;
			m_stagingBuffer.RequirementAlignmentInDeviceMem = memoryrequirement.alignment;
		}

		/* After created the buffer, time to allocate the requirement memory for it */
		// allocate a big devicememory to store these buffers
		{
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = m_stagingBuffer.RequirementSizeInDeviceMem;
			allocInfo.memoryTypeIndex = m_stagingBuffer.MemoryTypeIndex;

			DebugLog::EC(vkAllocateMemory(m_logicaldevice, &allocInfo, nullptr, &m_devicememory_staged));

			// bind the buffer at the beginning of 0 offset in the Device Memory
			vkBindBufferMemory(m_logicaldevice, m_stagingBuffer.buffer, m_devicememory_staged, 0);
		}
	}

	void ResourceManager::DeviceBufferInit_()
	{
		{
			VkBufferCreateInfo buffinfo{};
			buffinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			buffinfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // only be used from the graphics queue
			buffinfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			buffinfo.size = VulkanBufferObject::CurrentBufferTotalSize; // total sizes in the buffer
			vkCreateBuffer(m_logicaldevice, &buffinfo, nullptr, &m_mainBuffer.buffer);
		}

		// get all the memory requirements info about this buffer
		{
			VkMemoryRequirements memoryrequirement{};
			vkGetBufferMemoryRequirements(m_logicaldevice, m_mainBuffer.buffer, &memoryrequirement);
			m_mainBuffer.MemoryTypeIndex = Renderer::getInstance()->findMemoryType(memoryrequirement.memoryTypeBits,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT); // find the best heap in the GPU to store at
			m_mainBuffer.BufferTotalSize = VulkanBufferObject::CurrentBufferTotalSize;
			m_mainBuffer.RequirementSizeInDeviceMem = memoryrequirement.size;
			m_mainBuffer.RequirementAlignmentInDeviceMem = memoryrequirement.alignment;
		}

		/* After created the buffer, time to allocate the requirement memory for it */
		// allocate a big devicememory to store these buffers
		{
			VkMemoryAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			allocInfo.allocationSize = m_mainBuffer.RequirementSizeInDeviceMem;
			allocInfo.memoryTypeIndex = m_mainBuffer.MemoryTypeIndex;

			DebugLog::EC(vkAllocateMemory(m_logicaldevice, &allocInfo, nullptr, &m_devicememory_main));

			// bind the buffer at the beginning of 0 offset in the Device Memory
			vkBindBufferMemory(m_logicaldevice, m_mainBuffer.buffer, m_devicememory_main, 0);
		}
	}

	void ResourceManager::CopyBufferToDevice_()
	{
		// create the temp command pool
		VkCommandPool commandpool = VK_NULL_HANDLE;
		VkCommandPoolCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		createinfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT;
		createinfo.queueFamilyIndex = Renderer::getInstance()->GetQueueFamilyIndices().graphicsFamily;
		vkCreateCommandPool(m_logicaldevice, &createinfo, nullptr, &commandpool);

		// create the command buffer and start recording it
		VkCommandBuffer commandbuffer = VK_NULL_HANDLE;

		// allocate the temporary command buffer
		VkCommandBufferAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocinfo.commandPool = commandpool;
		allocinfo.commandBufferCount = 1;
		allocinfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vkAllocateCommandBuffers(m_logicaldevice, &allocinfo, &commandbuffer);

		// immediately start recording this command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		vkBeginCommandBuffer(commandbuffer, &beginInfo);

		// start the copy cmd
		// copy the damn buffer
		VkBufferCopy copyregion{};
		copyregion.srcOffset = 0;
		copyregion.dstOffset = 0;
		copyregion.size = VulkanBufferObject::CurrentBufferTotalSize;
		vkCmdCopyBuffer(commandbuffer, m_stagingBuffer.buffer, m_mainBuffer.buffer, 1, &copyregion);

		vkEndCommandBuffer(commandbuffer);

		// then submit this to the graphics queue to execute it
		// now execute the command buffer
		VkSubmitInfo submitinfo{};
		submitinfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitinfo.commandBufferCount = 1;
		submitinfo.pCommandBuffers = &commandbuffer;

		vkQueueSubmit(Renderer::getInstance()->GetGraphicQueue(), 1, &submitinfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(Renderer::getInstance()->GetGraphicQueue());

		// clean up
		vkDestroyCommandPool(m_logicaldevice, commandpool, nullptr);
		if (m_stagingBuffer.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_logicaldevice, m_stagingBuffer.buffer, nullptr);
			m_stagingBuffer.buffer = VK_NULL_HANDLE;
		}

		if (m_devicememory_staged != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_logicaldevice, m_devicememory_staged, nullptr);
			m_devicememory_staged = VK_NULL_HANDLE;
		}
	}

	void ResourceManager::Destroy()
	{
		for (auto &model : Models)
		{
			if (model != nullptr)
			{
				delete model;
			}
		}

		Models.clear();

		if (UBO != nullptr)
		{
			delete UBO;
			UBO = nullptr;
		}

		if (m_stagingBuffer.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_logicaldevice, m_stagingBuffer.buffer, nullptr);
			m_stagingBuffer.buffer = VK_NULL_HANDLE;
		}

		if (m_devicememory_staged != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_logicaldevice, m_devicememory_staged, nullptr);
			m_devicememory_staged = VK_NULL_HANDLE;
		}

		if (m_mainBuffer.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(m_logicaldevice, m_mainBuffer.buffer, nullptr);
			m_mainBuffer.buffer = VK_NULL_HANDLE;
		}

		if (m_devicememory_main != VK_NULL_HANDLE)
		{
			vkFreeMemory(m_logicaldevice, m_devicememory_main, nullptr);
			m_devicememory_main = VK_NULL_HANDLE;
		}
	}
}