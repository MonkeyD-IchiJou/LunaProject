#include "VulkanRenderer.h"
#include "DebugLog.h"

#include <array>

namespace luna
{
	VulkanRenderer::VulkanRenderer()
	{
		// straight away init the vulkan api instance

		SetUpInstanceLayersAndExtension_();
		SetUpVulkanDebug_();

		InitVulkanInstance_();
		InitVulkanDebug_();

		// vulkan logical device init

		SetUpLogicalDevice_();
		InitLogicalDevice_();
	}

	void VulkanRenderer::InitVulkanInstance_()
	{
		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Hello Luna";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName = "Luna";
		app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion = VK_API_VERSION_1_0;

		VkInstanceCreateInfo instance_create_info{};
		instance_create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instance_create_info.pApplicationInfo = &app_info;
		instance_create_info.enabledExtensionCount = (uint32_t)m_instance_exts.size();
		instance_create_info.ppEnabledExtensionNames = m_instance_exts.data();
#if _DEBUG
		instance_create_info.enabledLayerCount = (uint32_t)m_instance_layers.size();
		instance_create_info.ppEnabledLayerNames = m_instance_layers.data();
		instance_create_info.pNext = &m_debuginfo; // tell the instance about my debug layer has turned on
#endif

		DebugLog::EC(vkCreateInstance(&instance_create_info, nullptr, &m_vulkan_instance));

#if defined(__ANDROID__)
		DebugLog::printFF("init all the vulkan functions !!!!...");
		loadVulkanFunctions(m_vulkan_instance);
#endif
	}

	void VulkanRenderer::SetUpLogicalDevice_()
	{
		// get all the gpu information in this device
		m_physicaldevices.Init(m_vulkan_instance);

		// choose the first gpu
		int gpuChoose = 0;

		// get the queue family handle
		m_queuefamily_index = m_physicaldevices.findQueueFamilies(gpuChoose);
		if (!m_queuefamily_index.isComplete())
		{
			DebugLog::printFF("Cannot find suitable Queues");
		}

		// get all the info about this chosen gpu
		m_gpu = m_physicaldevices.getGPU(gpuChoose);
		m_gpu_properties = m_physicaldevices.getGPUProperties(gpuChoose);
		m_gpu_features = m_physicaldevices.getGPUDeviceFeatures(gpuChoose);
		m_gpu_memProperties = m_physicaldevices.getGPUMemoryProperties(gpuChoose);
	
		DebugLog::printFF("Physical Device: ");
		DebugLog::printFF(m_gpu_properties.deviceName);

#if _DEBUG
		/* print out all the device extensions only */
		{
			uint32_t deviceExtCount = 0;
			vkEnumerateDeviceExtensionProperties(m_gpu, nullptr, &deviceExtCount, nullptr);
			std::vector<VkExtensionProperties> extension_list(deviceExtCount);
			vkEnumerateDeviceExtensionProperties(m_gpu, nullptr, &deviceExtCount, extension_list.data());

			DebugLog::printL("\nDevice Extensions: ");
			for (auto i : extension_list)
			{
				DebugLog::print("\t");
				DebugLog::printL(i.extensionName);
			}

			DebugLog::print("\n");
		}

		// only in debug mode we can see the line
		m_required_features.fillModeNonSolid = VK_TRUE;

#endif // BUILD_ENABLE_VULKAN_DEBUG

		// swap chain extension support 
		m_device_exts.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}

	void VulkanRenderer::InitLogicalDevice_()
	{
		float queue_priorities[]{ 1.0f };
		VkDeviceQueueCreateInfo device_queue_create_info{};
		device_queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		device_queue_create_info.queueFamilyIndex = m_queuefamily_index.graphicsFamily; // will create one queue for this logical device based on this index
		device_queue_create_info.queueCount = 1;
		device_queue_create_info.pQueuePriorities = queue_priorities;

		VkDeviceCreateInfo device_info{};
		device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		device_info.queueCreateInfoCount = 1;
		device_info.pQueueCreateInfos = &device_queue_create_info;
		device_info.enabledExtensionCount = (uint32_t)m_device_exts.size();
		device_info.ppEnabledExtensionNames	 = m_device_exts.data();
		device_info.pEnabledFeatures = &m_required_features;

		// create the logical device

		if (vkCreateDevice(m_gpu, &device_info, nullptr, &m_logicaldevice) == VK_SUCCESS)
		{
			DebugLog::printFF("Created logical device ");
		}

		// get the queue from the device. queues are constructed when the device is created
		vkGetDeviceQueue(m_logicaldevice, m_queuefamily_index.graphicsFamily, 0, &m_graphic_queue);
	}

	void VulkanRenderer::DeInit_()
	{
		/* deinit the logical device */
		if (m_logicaldevice != VK_NULL_HANDLE)
		{
			vkDestroyDevice(m_logicaldevice, nullptr);
			m_logicaldevice = VK_NULL_HANDLE;
		}

		/* deinit the debug report */
		if (m_debugreport != VK_NULL_HANDLE)
		{
			PFN_vkDestroyDebugReportCallbackEXT fvkDestroyDebugReportCallbackEXT = 
				(PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(m_vulkan_instance, "vkDestroyDebugReportCallbackEXT");

			if (fvkDestroyDebugReportCallbackEXT == nullptr)
			{
				DebugLog::throwEx("Vulkan ERROR: Can't fetch debug function pointers");
			}

			fvkDestroyDebugReportCallbackEXT(m_vulkan_instance, m_debugreport, nullptr);
			m_debugreport = VK_NULL_HANDLE;
		}

		/* lastly, deinit my vulkan instance */
		if (m_vulkan_instance != VK_NULL_HANDLE)
		{
			vkDestroyInstance(m_vulkan_instance, nullptr);
			m_vulkan_instance = VK_NULL_HANDLE;
		}
	}

	const uint32_t VulkanRenderer::findMemoryType(const uint32_t & typeFilter, const VkMemoryPropertyFlags & properties) const
	{
		// find a memory type that is suitable for the buffer
		for (uint32_t i = 0; i < m_gpu_memProperties.memoryTypeCount; ++i)
		{
			auto check = (typeFilter & (1 << i)); // must be more than 0

			if (check)
			{
				if (((m_gpu_memProperties.memoryTypes[i].propertyFlags & properties) == properties))
				{
					return i;
				}
			}
		}

		DebugLog::throwEx("failed to find suitable memory type");

		return 0;
	}

	const VkFormat VulkanRenderer::findSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling & tiling, const VkFormatFeatureFlags & features) const
	{
		for (VkFormat format : candidates)
		{
			VkFormatProperties props{};
			vkGetPhysicalDeviceFormatProperties(m_gpu, format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
			{
				return format;
			}
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
			{
				return format;
			}
		}

		DebugLog::throwEx("failed to find supported format!");
		return VkFormat();
	}


	/* Physical Device Struct */

	void VulkanRenderer::PhysicalDevice::Init(const VkInstance & instance)
	{
		vkEnumeratePhysicalDevices(instance, &TotalNumOfGPUs, nullptr);
		//if (TotalNumOfGPUs < 0) { DebugLog::throwEx("Vulkan ERROR: Cannot find any gpu"); }
		GPUs.resize(TotalNumOfGPUs);
		vkEnumeratePhysicalDevices(instance, &TotalNumOfGPUs, GPUs.data());

		// store the details also
		GPUs_Properties.resize(TotalNumOfGPUs);
		GPUs_MemoryProperties.resize(TotalNumOfGPUs);
		GPUs_DeviceFeatures.resize(TotalNumOfGPUs);
		for (uint32_t i = 0; i < TotalNumOfGPUs; ++i)
		{
			vkGetPhysicalDeviceProperties(GPUs[i], &GPUs_Properties[i]);
			vkGetPhysicalDeviceMemoryProperties(GPUs[i], &GPUs_MemoryProperties[i]);
			vkGetPhysicalDeviceFeatures(GPUs[i], &GPUs_DeviceFeatures[i]);
		}
	}

	QueueFamilyIndices VulkanRenderer::PhysicalDevice::findQueueFamilies(const uint32_t & whichOne)
	{
		QueueFamilyIndices indices{};
		auto gpu = GPUs[whichOne];

		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, nullptr);
		std::vector<VkQueueFamilyProperties> queuefamily_property_list(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyCount, queuefamily_property_list.data());

		int i = 0;
		for (const auto& queueFamily : queuefamily_property_list)
		{
			if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				indices.graphicsFamily = i;
			}

			if (indices.isComplete())
			{
				break;
			}

			i++;
		}

		return indices;
	}

}
