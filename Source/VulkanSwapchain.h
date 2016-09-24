#ifndef VULKAN_SWAPCHAIN_H
#define VULKAN_SWAPCHAIN_H

#include "platform.h"

#include <vector>

namespace luna
{
	struct SwapChainBuffer
	{
		VkImage image = VK_NULL_HANDLE;
		VkImageView imageview = VK_NULL_HANDLE;
	};

	class VulkanSwapchain
	{
	public:
		VulkanSwapchain();
		~VulkanSwapchain();

		/* create the swapchain and its buffers 
		* @param width Pointer to the width of the swapchain (may be adjusted to fit the requirements of the swapchain)
		* @param height Pointer to the height of the swapchain (may be adjusted to fit the requirements of the swapchain)
		*/
		void CreateResources(uint32_t width, uint32_t height);

		/* Acquires the next image in the swap chain */
		/* The function will always wait until the next image has been acquired by setting timeout to UINT64_MAX */
		VkResult AcquireNextImage(VkSemaphore presentCompletSemaphore, uint32_t *imageIndex);

		/* Queue an image for presentation */
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

		/* destroy the swapchain and its buffers */
		void Destroy();

		auto getSwapchain() const { return m_swapchain; }
		auto getColorFormat() const { return m_colorformat; }
		auto getColorSpace() const { return m_colorspace; }
		auto getImageCount() const { return m_imagecount; }

		std::vector<SwapChainBuffer> m_buffers;

	private:
		/* necessary handle from renderer */
		VkInstance m_vulkanInstance = VK_NULL_HANDLE;
		VkDevice m_logicalDevice = VK_NULL_HANDLE;
		VkPhysicalDevice m_gpu = VK_NULL_HANDLE;

		/* surface info */
		VkSurfaceKHR m_surface = VK_NULL_HANDLE;
		VkFormat m_colorformat = VK_FORMAT_UNDEFINED;
		VkColorSpaceKHR m_colorspace = VK_COLOR_SPACE_MAX_ENUM_KHR;

		/* swapchain datas */
		VkSwapchainKHR m_swapchain = VK_NULL_HANDLE;
		uint32_t m_imagecount = 0;
		std::vector<VkImage> m_images;

		uint32_t m_queueIndex = 0;
	};
}

#endif

