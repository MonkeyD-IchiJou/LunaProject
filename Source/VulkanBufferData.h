#ifndef VULKAN_BUFFER_DATA_H
#define VULKAN_BUFFER_DATA_H

#include "platform.h"

namespace luna
{
	struct VulkanBufferData
	{
		VkDeviceSize BufferTotalSize = 0;
		VkDeviceSize RequirementSizeInDeviceMem = 0;
		VkDeviceSize RequirementAlignmentInDeviceMem = 0;
		uint32_t MemoryTypeIndex = 0;
		VkBuffer buffer = VK_NULL_HANDLE;
	};
}

#endif
