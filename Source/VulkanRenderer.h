#ifndef VULKANRENDERER_H
#define VULKANRENDERER_H

#include "platform.h"
#include "QueueFamilyIndices.h"
#include <vector>
#include <string>

namespace luna
{
	/* Init vulkan api, logical device and get the GPU handle, rendering tools */
	class VulkanRenderer
	{
	public:
		virtual void CreateResources() = 0;
		virtual void CleanUpResources() = 0;

		inline VkInstance GetVulkanInstance() const { return m_vulkan_instance; }
		inline VkDevice GetLogicalDevice() const { return m_logicaldevice; }
		inline VkPhysicalDevice GetGPU() const { return m_gpu; }
		inline VkQueue GetGraphicQueue() const { return m_graphic_queue; }
		inline const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_queuefamily_index; }
		inline const VkPhysicalDeviceProperties& GetGPUProperties() const { return m_gpu_properties; }
		inline const VkPhysicalDeviceMemoryProperties& GetGPUMemoryProperties() const { return m_gpu_memProperties; }

		const uint32_t findMemoryType(const uint32_t& typeFilter, const VkMemoryPropertyFlags& properties) const;
		const VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, const VkImageTiling& tiling, const VkFormatFeatureFlags& features) const;

	protected:
		VulkanRenderer();
		virtual ~VulkanRenderer() {};

		void SetUpInstanceLayersAndExtension_();
		void SetUpVulkanDebug_();

		void InitVulkanInstance_();
		void InitVulkanDebug_();

		void SetUpLogicalDevice_();
		void InitLogicalDevice_();

		void DeInit_();

	protected:
		VkInstance m_vulkan_instance = VK_NULL_HANDLE;
		VkDebugReportCallbackEXT m_debugreport = VK_NULL_HANDLE;

		VkPhysicalDevice m_gpu = VK_NULL_HANDLE;
		VkPhysicalDeviceProperties m_gpu_properties = {};
		VkPhysicalDeviceFeatures m_gpu_features = {};
		VkPhysicalDeviceMemoryProperties m_gpu_memProperties = {};
		VkPhysicalDeviceFeatures m_required_features = {}; /* features that is required for the app */

		VkDevice m_logicaldevice = VK_NULL_HANDLE;
		VkQueue m_graphic_queue = VK_NULL_HANDLE;

		std::vector<const char*> m_instance_exts;
		std::vector<const char*> m_instance_layers;
		std::vector<const char*> m_device_exts;
		VkDebugReportCallbackCreateInfoEXT m_debuginfo{};

		// queue for submitting later for rendering and display
		QueueFamilyIndices m_queuefamily_index{};

		struct PhysicalDevice
		{
			uint32_t TotalNumOfGPUs = 0;
			std::vector<VkPhysicalDevice> GPUs;
			std::vector<VkPhysicalDeviceProperties> GPUs_Properties;
			std::vector<VkPhysicalDeviceMemoryProperties> GPUs_MemoryProperties;
			std::vector<VkPhysicalDeviceFeatures> GPUs_DeviceFeatures;

			void Init(const VkInstance& instance);

			inline uint32_t getTotalNumOfGPUs() const { return TotalNumOfGPUs; }
			inline VkPhysicalDevice getGPU(const uint32_t &whichOne) const { return GPUs[whichOne]; }
			inline VkPhysicalDeviceProperties getGPUProperties(const uint32_t &whichOne) const { return GPUs_Properties[whichOne]; }
			inline VkPhysicalDeviceMemoryProperties	getGPUMemoryProperties(const uint32_t &whichOne) const { return GPUs_MemoryProperties[whichOne]; }
			inline VkPhysicalDeviceFeatures getGPUDeviceFeatures(const uint32_t &whichOne) const { return GPUs_DeviceFeatures[whichOne]; }

			QueueFamilyIndices findQueueFamilies(const uint32_t& whichOne);

		} m_physicaldevices;
	};
}

#endif

