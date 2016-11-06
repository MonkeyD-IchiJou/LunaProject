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

		// when window size changes
		void RecreateSwapChain();

		/* Acquires the next image in the swap chain */
		/* The function will always wait until the next image has been acquired by setting timeout to UINT64_MAX */
		VkResult AcquireNextImage(VkSemaphore presentCompletSemaphore, uint32_t *imageIndex);

		/* Queue an image for presentation */
		VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

		/* destroy the swapchain and its buffers */
		void Destroy();

		VkSwapchainKHR getSwapchain() const { return m_swapchain; }
		VkFormat getColorFormat() const { return m_colorformat; }
		VkColorSpaceKHR getColorSpace() const { return m_colorspace; }
		uint32_t getImageCount() const { return m_imagecount; }
		VkExtent2D getExtent() const { return m_swapchainExtent; }

		std::vector<SwapChainBuffer> m_buffers;

	private:
		/* create the swapchain and its buffers */
		void Init_();

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
		VkExtent2D m_swapchainExtent = {};
		std::vector<VkImage> m_images;

		uint32_t m_queueIndex = 0;
	};
}

#endif

