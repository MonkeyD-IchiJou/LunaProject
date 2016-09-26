#ifndef VULKAN_BUFFER_OBJ_H
#define VULKAN_BUFFER_OBJ_H

#include "platform.h"

namespace luna
{
	class VulkanBufferObject
	{
	public:
		VulkanBufferObject();
		virtual ~VulkanBufferObject();

		/* current total size in this buffer */
		static VkDeviceSize CurrentBufferTotalSize;
	};
}

#endif
