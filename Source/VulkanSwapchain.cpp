#include "VulkanSwapchain.h"
#include "DebugLog.h"
#include "Renderer.h"
#include "WinNative.h"

namespace luna
{

	VulkanSwapchain::VulkanSwapchain()
	{
		/* get all the handles from the renderer */
		Renderer* r = Renderer::getInstance();
		m_vulkanInstance = r->GetVulkanInstance();
		m_gpu = r->GetGPU();
		m_logicalDevice = r->GetLogicalDevice();
		m_queueIndex = r->GetQueueFamilyIndices().graphicsFamily;
		m_surface = WinNative::getInstance()->getSurface();
	}

	VulkanSwapchain::~VulkanSwapchain()
	{
	}

	void VulkanSwapchain::CreateResources(uint32_t width, uint32_t height)
	{
		/* check for surface available or not */
		if (m_surface == VK_NULL_HANDLE)
		{
			DebugLog::throwEx("surface not avalable for swap chain");
			return;
		}

		/* check for wsi supported or not */
		VkBool32 WSI_supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(m_gpu, m_queueIndex, m_surface, &WSI_supported);
		if (!WSI_supported)
		{
			DebugLog::throwEx("WSI is not supported");
			return;
		}

		/* get the color formats and color space */
		{
			uint32_t surfaceFormatCount = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &surfaceFormatCount, nullptr);
			if (surfaceFormatCount == 0)
			{
				DebugLog::throwEx("surface formats missing !");
				return;
			}
			std::vector<VkSurfaceFormatKHR> surface_formats_list(surfaceFormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu, m_surface, &surfaceFormatCount, surface_formats_list.data());

			if (surface_formats_list[0].format == VK_FORMAT_UNDEFINED)
			{
				m_colorformat = VK_FORMAT_B8G8R8A8_UNORM;
				m_colorspace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
			}
			else
			{
				// for now, always choose the first one
				m_colorformat = surface_formats_list[0].format;
				m_colorspace = surface_formats_list[0].colorSpace;
			}
		}

		/* get the surface capabilities and adjust the width and height */
		VkSurfaceCapabilitiesKHR surface_capabilities{};
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_gpu, m_surface, &surface_capabilities);
		VkExtent2D swapchainExtent = {};

		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (surface_capabilities.currentExtent.width == (uint32_t)-1)
		{
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = width;
			swapchainExtent.height = height;
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = surface_capabilities.currentExtent;
			//*width = surface_capabilities.currentExtent.width;
			//*height = surface_capabilities.currentExtent.height;
		}

		// Determine the number of images
		if (m_imagecount < surface_capabilities.minImageCount + 1)
		{
			m_imagecount = surface_capabilities.minImageCount + 1;
		}
		// if is 0 .. means unlimited swapchain image
		if (surface_capabilities.maxImageCount > 0) 
		{
			if (m_imagecount > surface_capabilities.maxImageCount)
			{
				m_imagecount = surface_capabilities.maxImageCount;
			}
		}

		/* get all the available present mode */
		uint32_t presentModeCount = 0;
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &presentModeCount, nullptr);
		std::vector<VkPresentModeKHR> m_presentmode_list(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(m_gpu, m_surface, &presentModeCount, m_presentmode_list.data());
		// default present mode is FIFO
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; 
		// find the best present mode --> MAILBOX 
		for (auto i : m_presentmode_list)
		{
			if (i == VK_PRESENT_MODE_MAILBOX_KHR)
				presentMode = i;
		}

		/* swap chain creation start */
		VkSwapchainKHR oldSwapChain = m_swapchain;

		VkSwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.sType	= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.surface = m_surface;
		swapchain_create_info.minImageCount = m_imagecount;
		swapchain_create_info.imageFormat = m_colorformat;
		swapchain_create_info.imageColorSpace = m_colorspace;
		swapchain_create_info.imageExtent = swapchainExtent;
		swapchain_create_info.imageArrayLayers = 1; // how many layer does the images have (vr need 2)
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount	= 0;
		swapchain_create_info.pQueueFamilyIndices = nullptr;
		swapchain_create_info.preTransform = surface_capabilities.currentTransform;
		swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode = presentMode;
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.oldSwapchain = oldSwapChain; // useful when resizing the windows

		DebugLog::EC(vkCreateSwapchainKHR(m_logicalDevice, &swapchain_create_info, nullptr, &m_swapchain));

		// If an existing sawp chain is re-created, destroy the old swap chain
		// This also cleans up all the presentable images
		// prevent memory leak 
		if (oldSwapChain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < m_imagecount; i++)
			{
				vkDestroyImageView(m_logicalDevice, m_buffers[i].imageview, nullptr);
			}
			vkDestroySwapchainKHR(m_logicalDevice, oldSwapChain, nullptr);
			oldSwapChain = VK_NULL_HANDLE;
		}

		/* buffers creation start below */

		// get the number of images count and all the images in the swap chain
		vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &m_imagecount, nullptr);
		m_images.resize(m_imagecount);
		vkGetSwapchainImagesKHR(m_logicalDevice, m_swapchain, &m_imagecount, m_images.data());

		m_buffers.resize(m_imagecount);
		for (uint32_t i = 0; i < m_imagecount; ++i)
		{
			m_buffers[i].image = m_images[i];

			VkImageViewCreateInfo imgview_createInfo{};
			imgview_createInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imgview_createInfo.image								= m_buffers[i].image;
			imgview_createInfo.viewType								= VK_IMAGE_VIEW_TYPE_2D;
			imgview_createInfo.format								= m_colorformat;
			imgview_createInfo.components.r							= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgview_createInfo.components.g							= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgview_createInfo.components.b							= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgview_createInfo.components.a							= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgview_createInfo.subresourceRange.aspectMask			= VK_IMAGE_ASPECT_COLOR_BIT;
			imgview_createInfo.subresourceRange.baseMipLevel		= 0;
			imgview_createInfo.subresourceRange.levelCount			= 1;
			imgview_createInfo.subresourceRange.baseArrayLayer		= 0;
			imgview_createInfo.subresourceRange.layerCount			= 1;

			DebugLog::EC(vkCreateImageView(m_logicalDevice, &imgview_createInfo, nullptr, &m_buffers[i].imageview));
		}
	}

	VkResult VulkanSwapchain::AcquireNextImage(VkSemaphore presentCompletSemaphore, uint32_t * imageIndex)
	{
		return vkAcquireNextImageKHR(m_logicalDevice, m_swapchain, UINT64_MAX, presentCompletSemaphore, VK_NULL_HANDLE, imageIndex);
	}

	VkResult VulkanSwapchain::QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore)
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &m_swapchain;
		presentInfo.pImageIndices = &imageIndex;
		// Check if a wait semaphore has been specified to wait for before presenting the image
		if (waitSemaphore != VK_NULL_HANDLE)
		{
			presentInfo.waitSemaphoreCount = 1;
			presentInfo.pWaitSemaphores = &waitSemaphore; // wait until render finish
		}

		return vkQueuePresentKHR(queue, &presentInfo);
	}

	void VulkanSwapchain::Destroy()
	{
		/* shld only called when the window close or app exit */
		if (m_swapchain != VK_NULL_HANDLE)
		{
			for (uint32_t i = 0; i < m_imagecount; i++)
			{
				vkDestroyImageView(m_logicalDevice, m_buffers[i].imageview, nullptr);
			}

			vkDestroySwapchainKHR(m_logicalDevice, m_swapchain, nullptr);
			m_swapchain = VK_NULL_HANDLE;
		}
	}
}
