#ifndef VULKAN_LUNATEXTURE2D_H
#define VULKAN_LUNATEXTURE2D_H

#include "VulkanImageBufferObject.h"

namespace luna
{
	class VulkanTexture2D : public VulkanImageBufferObject
	{
	public:
		// texture data read from files
		VulkanTexture2D(const std::string& filename);

		// attachment texture to be read/write later
		VulkanTexture2D(const uint32_t& width, const uint32_t& height, 
			const VkFormat& format, const VkImageUsageFlags& usage, const VkImageAspectFlags& aspectMask, const VkImageLayout& imagelayout);

		virtual ~VulkanTexture2D();
		
	private:
		VkDeviceMemory m_devicememory = VK_NULL_HANDLE;
	};
}

#endif
