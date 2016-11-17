#include "CommandBufferPacket.h"
#include "DebugLog.h"

namespace luna
{
	CommandBufferPacket::CommandBufferPacket(const uint32_t& queueFamilyIndex, const VkDevice logicaldevice)
	{
		{
			VkCommandPoolCreateInfo commandPool_createinfo{};
			commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPool_createinfo.queueFamilyIndex = queueFamilyIndex;
			DebugLog::EC(vkCreateCommandPool(logicaldevice, &commandPool_createinfo, nullptr, &primary_commandpool));

			// primary command buffers creation, used the same command pool from renderer
			VkCommandBufferAllocateInfo buffer_allocateInfo{};
			buffer_allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			buffer_allocateInfo.commandPool = primary_commandpool;
			buffer_allocateInfo.commandBufferCount = 1;
			DebugLog::EC(vkAllocateCommandBuffers(logicaldevice, &buffer_allocateInfo, &offscreen_cmdbuffer));
		}

		{
			// secondary command buffer creation
			// it is able to reset the command buffer
			VkCommandPoolCreateInfo commandPool_createinfo{};
			commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPool_createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPool_createinfo.queueFamilyIndex = queueFamilyIndex;
			DebugLog::EC(vkCreateCommandPool(logicaldevice, &commandPool_createinfo, nullptr, &secondary_commandpool_thrd0));

			VkCommandBufferAllocateInfo buffer_allocateInfo{};
			buffer_allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			buffer_allocateInfo.commandPool = secondary_commandpool_thrd0;
			buffer_allocateInfo.commandBufferCount = 1;
			DebugLog::EC(vkAllocateCommandBuffers(logicaldevice, &buffer_allocateInfo, &skybox_secondary_cmdbuff));
			DebugLog::EC(vkAllocateCommandBuffers(logicaldevice, &buffer_allocateInfo, &transferdata_secondary_cmdbuff));
			DebugLog::EC(vkAllocateCommandBuffers(logicaldevice, &buffer_allocateInfo, &lightingsubpass_secondary_cmdbuff));
		}

		{
			VkCommandPoolCreateInfo commandPool_createinfo{};
			commandPool_createinfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			commandPool_createinfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			commandPool_createinfo.queueFamilyIndex = queueFamilyIndex;
			DebugLog::EC(vkCreateCommandPool(logicaldevice, &commandPool_createinfo, nullptr, &secondary_commandpool_thrd1));

			VkCommandBufferAllocateInfo buffer_allocateInfo{};
			buffer_allocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			buffer_allocateInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			buffer_allocateInfo.commandPool = secondary_commandpool_thrd1;
			buffer_allocateInfo.commandBufferCount = 1;
			DebugLog::EC(vkAllocateCommandBuffers(logicaldevice, &buffer_allocateInfo, &gbuffer_secondary_cmdbuff));
		}
	}

	CommandBufferPacket::~CommandBufferPacket()
	{
	}

	void CommandBufferPacket::Destroy(const VkDevice logicaldevice)
	{
		if (primary_commandpool != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(logicaldevice, primary_commandpool, nullptr);
			primary_commandpool = VK_NULL_HANDLE;
		}

		if (secondary_commandpool_thrd0 != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(logicaldevice, secondary_commandpool_thrd0, nullptr);
			secondary_commandpool_thrd0 = VK_NULL_HANDLE;
		}

		if (secondary_commandpool_thrd1 != VK_NULL_HANDLE)
		{
			vkDestroyCommandPool(logicaldevice, secondary_commandpool_thrd1, nullptr);
			secondary_commandpool_thrd1 = VK_NULL_HANDLE;
		}
	}
}
