#ifndef VULKAN_BUFFER_DATA_H
#define VULKAN_BUFFER_DATA_H

#include "platform.h"

namespace luna
{
	struct VulkanBufferData
	{
		VkDeviceSize BufferTotalSize = 0;
		uint32_t MemoryTypeIndex = 0;
		VkDeviceSize RequirementSizeInDeviceMem = 0;
		VkDeviceSize RequirementAlignmentInDeviceMem = 0;
		VkBuffer buffer = VK_NULL_HANDLE;
	};

	struct VulkanImageBufferData
	{
		VkDeviceSize BufferTotalSize = 0;
		uint32_t MemoryTypeIndex = 0;
		VkDeviceSize RequirementSizeInDeviceMem = 0;
		VkDeviceSize RequirementAlignmentInDeviceMem = 0;
		VkImage buffer = VK_NULL_HANDLE;
		VkFormat format = VK_FORMAT_UNDEFINED;
		VkImageTiling tiling = VK_IMAGE_TILING_MAX_ENUM;
		VkImageUsageFlags imageusage = VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
	};
}

#endif
