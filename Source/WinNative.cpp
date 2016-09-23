#include "WinNative.h"
#include "Renderer.h"
#include "DebugLog.h"

namespace luna
{
	std::once_flag WinNative::m_sflag{};
	WinNative* WinNative::m_instance = nullptr;

	WinNative::WinNative() : 
		m_surface_size_x(1080), 
		m_surface_size_y(720), 
		m_win_pos_x(800), 
		m_win_pos_y(100),
		m_win_name("Luna")
	{			
		/* get the reference of the renderer */
		renderer_handle = Renderer::getInstance();

		// init the os window
		InitOSWindow_();	 

		// surface inits and query all informations about it
		InitOSWindowSurface_();
		QuerySurfaceInfo_();

		// swap chain init with surface info
		InitSwapChain_();
	}

	void WinNative::UpdateOSWin()
	{
		// update the os window
		UpdateOSWindow_();
	}

	void WinNative::QuerySurfaceInfo_()
	{
		// get the current gpu first
		auto gpu = renderer_handle->GetGPU();

		// check for wsi supported or not
		VkBool32 WSI_supported = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(gpu, renderer_handle->GetQueueFamilyIndices().graphicsFamily, m_surface, &WSI_supported);
		if (!WSI_supported)
		{
			DebugLog::throwEx("WSI is not supported");
			return;
		}

		// get the surface capabilities
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, m_surface, &m_surface_capabilities);
		if (m_surface_capabilities.currentExtent.width < UINT32_MAX)
		{
			m_surface_size_x = m_surface_capabilities.currentExtent.width;
			m_surface_size_y = m_surface_capabilities.currentExtent.height;
		}

		// get the surface formats
		{
			uint32_t surfaceFormatCount = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_surface, &surfaceFormatCount, nullptr);

			if (surfaceFormatCount == 0)
			{
				DebugLog::throwEx("surface formats missing !");
				return;
			}

			std::vector<VkSurfaceFormatKHR> surface_formats_list(surfaceFormatCount);
			vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, m_surface, &surfaceFormatCount, surface_formats_list.data());

			if (surface_formats_list[0].format == VK_FORMAT_UNDEFINED)
			{
				m_surface_formats.format = VK_FORMAT_B8G8R8A8_UNORM;
				m_surface_formats.colorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
			}
			else
			{
				// for now, always choose the first one
				m_surface_formats = surface_formats_list[0];
			}
		}

		// get all the available present mode
		{
			uint32_t presentModeCount = 0;
			vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, m_surface, &presentModeCount, nullptr);
			m_presentmode_list.resize(presentModeCount);
			vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, m_surface, &presentModeCount, m_presentmode_list.data());
		}

		// choose the correct extent
		{
			m_swapchain_extent.width = m_surface_size_x;
			m_swapchain_extent.height = m_surface_size_y;
		}
	}

	void WinNative::InitSwapChain_()
	{
		if (m_swapchain_image_count < m_surface_capabilities.minImageCount + 1)
		{
			m_swapchain_image_count = m_surface_capabilities.minImageCount + 1;
		}

		// if is 0 .. means unlimited swapchain image
		if (m_surface_capabilities.maxImageCount > 0) 
		{
			if (m_swapchain_image_count > m_surface_capabilities.maxImageCount) m_swapchain_image_count = m_surface_capabilities.maxImageCount;
		}

		// default present mode is FIFO
		VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR; 
		// find the best present mode --> MAILBOX 
		for (auto i : m_presentmode_list)
		{
			if (i == VK_PRESENT_MODE_MAILBOX_KHR)
				presentMode = i;
		}

		VkSwapchainKHR oldSwapChain = m_swapchain;

		VkSwapchainCreateInfoKHR swapchain_create_info{};
		swapchain_create_info.sType	= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchain_create_info.surface = m_surface;
		swapchain_create_info.minImageCount = m_swapchain_image_count;
		swapchain_create_info.imageFormat = m_surface_formats.format;
		swapchain_create_info.imageColorSpace = m_surface_formats.colorSpace;
		swapchain_create_info.imageExtent = m_swapchain_extent;
		swapchain_create_info.imageArrayLayers = 1; // how many layer does the images have (vr need 2)
		swapchain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_create_info.queueFamilyIndexCount	= 0;
		swapchain_create_info.pQueueFamilyIndices = nullptr;
		swapchain_create_info.preTransform = m_surface_capabilities.currentTransform;
		swapchain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		swapchain_create_info.presentMode = presentMode;
		swapchain_create_info.clipped = VK_TRUE;
		swapchain_create_info.oldSwapchain = oldSwapChain; // useful when resizing the windows
	
		DebugLog::EC(vkCreateSwapchainKHR(renderer_handle->GetLogicalDevice(), &swapchain_create_info, nullptr, &m_swapchain));
	
		// get the number of images count
		vkGetSwapchainImagesKHR(renderer_handle->GetLogicalDevice(), m_swapchain, &m_swapchain_image_count, nullptr);

		if (oldSwapChain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(renderer_handle->GetLogicalDevice(), oldSwapChain, nullptr);
		}
	}

	void WinNative::InitSwapChainImages_()
	{
		// Obtain all the images from the swapchain 
		m_swapchain_images.resize(m_swapchain_image_count);
		vkGetSwapchainImagesKHR(renderer_handle->GetLogicalDevice(), m_swapchain, &m_swapchain_image_count, m_swapchain_images.data());
	}

	void WinNative::InitSwapChainImageViews_()
	{
		// create the image views for the swap chain image
		m_swapchain_images_views.resize(m_swapchain_image_count);

		for (uint32_t i = 0; i < m_swapchain_image_count; ++i)
		{
			VkImageViewCreateInfo imgview_createInfo{};
			imgview_createInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imgview_createInfo.image								= m_swapchain_images[i];
			imgview_createInfo.viewType								= VK_IMAGE_VIEW_TYPE_2D;
			imgview_createInfo.format								= m_surface_formats.format;
			imgview_createInfo.components.r							= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgview_createInfo.components.g							= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgview_createInfo.components.b							= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgview_createInfo.components.a							= VK_COMPONENT_SWIZZLE_IDENTITY;
			imgview_createInfo.subresourceRange.aspectMask			= VK_IMAGE_ASPECT_COLOR_BIT;
			imgview_createInfo.subresourceRange.baseMipLevel		= 0;
			imgview_createInfo.subresourceRange.levelCount			= 1;
			imgview_createInfo.subresourceRange.baseArrayLayer		= 0;
			imgview_createInfo.subresourceRange.layerCount			= 1;

			DebugLog::EC(vkCreateImageView(renderer_handle->GetLogicalDevice(), &imgview_createInfo, nullptr, &m_swapchain_images_views[i]));
		}
	}

	void WinNative::DeInitOSWindowSurface_()
	{
		if (m_surface != VK_NULL_HANDLE)
		{
			vkDestroySurfaceKHR(renderer_handle->GetVulkanInstance(), m_surface, nullptr);
			m_surface = VK_NULL_HANDLE;
		}
	}

	void WinNative::DeInitSwapChain_()
	{
		if (m_swapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(renderer_handle->GetLogicalDevice(), m_swapchain, nullptr);
			m_swapchain = VK_NULL_HANDLE;
		}
	}
}
