#include "Renderer.h"
#include "DebugLog.h"

#include <sstream>

namespace luna
{
	void VulkanRenderer::SetUpInstanceLayersAndExtension_()
	{
#if _DEBUG
		/* just print out the number of extension available for the instance only */
		{
			uint32_t instanceExtCount = 0;
			vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtCount, nullptr);
			std::vector<VkExtensionProperties> extension_list(instanceExtCount);
			vkEnumerateInstanceExtensionProperties(nullptr, &instanceExtCount, extension_list.data());

			DebugLog::printL("Instance Extensions: ");

			for (auto i : extension_list)
			{
				DebugLog::print("\t");
				DebugLog::printL( i.extensionName );
			}

			DebugLog::print("\n");
		}
#endif // BUILD_ENABLE_VULKAN_DEBUG

		m_instance_exts.push_back(VK_KHR_SURFACE_EXTENSION_NAME);
		m_instance_exts.push_back(PLATFORM_SURFACE_EXTENSION_NAME);
	}

#if _DEBUG
	/* Only when in debug mode, then i will allow these to happen */

	static VKAPI_ATTR VkBool32 VKAPI_CALL
		VulkanDebugCallback(VkDebugReportFlagsEXT msg_flags, VkDebugReportObjectTypeEXT obj_type, uint64_t src_obj,
			size_t location, int32_t msg_code, const char* layer_prefix, const char* msg, void* user_data)
	{
		if (msg_flags &  VK_DEBUG_REPORT_WARNING_BIT_EXT)
		{
			DebugLog::printL(msg);
		}

		if (msg_flags &  VK_DEBUG_REPORT_ERROR_BIT_EXT)
		{
			std::ostringstream stream;
			stream << "(VKDBG) ERROR | @[" << layer_prefix << "]: " << msg << std::endl;

#ifdef _WIN32
			MessageBox(NULL, stream.str().c_str(), "Vulkan Error!", 0);
#endif
			DebugLog::throwEx(stream.str().c_str());

			return true;
		}

		return false;
	}

	void VulkanRenderer::SetUpVulkanDebug_()
	{
		// For reference
		// Instance layers
		{
			uint32_t layer_count = 0;
			vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
			std::vector<VkLayerProperties> layer_property_list(layer_count);
			vkEnumerateInstanceLayerProperties(&layer_count, layer_property_list.data());

			// print for debug purposes
			DebugLog::printL("Instance Layers: ");
			for (auto &i : layer_property_list)
			{
				DebugLog::print('\t');
				DebugLog::printL(i.layerName);
			}
			DebugLog::print('\n');
		}

		m_debuginfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
		m_debuginfo.pfnCallback = VulkanDebugCallback;
		m_debuginfo.flags =
			VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

		// store the layers extensions for later vulkan Instance creation
		m_instance_layers.push_back("VK_LAYER_GOOGLE_threading");
		m_instance_layers.push_back("VK_LAYER_LUNARG_parameter_validation");
		m_instance_layers.push_back("VK_LAYER_LUNARG_object_tracker");
		m_instance_layers.push_back("VK_LAYER_LUNARG_image");
		m_instance_layers.push_back("VK_LAYER_LUNARG_core_validation"); // this bastard cause memory leak 
		m_instance_layers.push_back("VK_LAYER_LUNARG_swapchain");
		m_instance_layers.push_back("VK_LAYER_GOOGLE_unique_objects");
		m_instance_exts.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
	}

	void VulkanRenderer::InitVulkanDebug_()
	{
		PFN_vkCreateDebugReportCallbackEXT fvkCreateDebugReportCallbackEXT = 
			(PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(m_vulkan_instance, "vkCreateDebugReportCallbackEXT");

		if (fvkCreateDebugReportCallbackEXT == nullptr)
		{
			DebugLog::throwEx("Vulkan ERROR: Can't fetch debug function pointers");
		}

		fvkCreateDebugReportCallbackEXT(m_vulkan_instance, &m_debuginfo, nullptr, &m_debugreport);
	}

#else

	void VulkanRenderer::SetUpVulkanDebug_()
	{
	}

	void VulkanRenderer::InitVulkanDebug_()
	{
	}

#endif

}
