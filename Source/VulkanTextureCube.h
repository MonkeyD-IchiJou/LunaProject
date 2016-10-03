#ifndef VULKAN_TEXTURE_CUBE_H
#define VULKAN_TEXTURE_CUBE_H

#include "VulkanImageBufferObject.h"

namespace luna
{
	class VulkanTextureCube :
		public VulkanImageBufferObject
	{
	public:
		VulkanTextureCube(const std::string& filename);
		virtual ~VulkanTextureCube();

	private:
		VkDeviceMemory m_devicememory = VK_NULL_HANDLE;
	};
}

#endif
