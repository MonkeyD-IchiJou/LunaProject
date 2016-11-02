#ifndef VULKAN_TEXTURELUNA_ARRAY_H
#define VULKAN_TEXTURELUNA_ARRAY_H

#include "VulkanImageBufferObject.h"

namespace luna
{
	class VulkanTextureArray2D :
		public VulkanImageBufferObject
	{
	public:
		VulkanTextureArray2D(const std::string& filename);
		virtual ~VulkanTextureArray2D();

	private:
		VkDeviceMemory m_devicememory = VK_NULL_HANDLE;
		uint32_t m_layers = 0;
	};
}

#endif

