#ifndef DEBUG_LOG_H
#define DEBUG_LOG_H

#include "platform.h"

#if _DEBUG
#include <iostream>
#include <string>
#include <cstdarg>
#endif //BUILD_ENABLE_DEBUG

namespace luna
{
	class DebugLog
	{
	public:
		static void printF(const char* pMessage, ...);

		static void throwEx(const char* format)
		{
#if _DEBUG
			throw std::runtime_error(format);
#endif //BUILD_ENABLE_DEBUG
		}

		static void EC(VkResult result)
		{
#if _DEBUG
			if (result != VK_SUCCESS)
			{
				switch (result)
				{
				case VK_ERROR_OUT_OF_HOST_MEMORY:
					printF( "VK_ERROR_OUT_OF_HOST_MEMORY \n" );
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					printF( "VK_ERROR_OUT_OF_DEVICE_MEMORY \n" );
					break;
				case VK_ERROR_INITIALIZATION_FAILED:
					printF( "VK_ERROR_INITIALIZATION_FAILED \n" );
					break;
				case VK_ERROR_DEVICE_LOST:
					printF( "VK_ERROR_DEVICE_LOST \n" );
					break;
				case VK_ERROR_MEMORY_MAP_FAILED:
					printF( "VK_ERROR_MEMORY_MAP_FAILED \n" );
					break;
				case VK_ERROR_LAYER_NOT_PRESENT:
					printF( "VK_ERROR_LAYER_NOT_PRESENT \n" );
					break;
				case VK_ERROR_EXTENSION_NOT_PRESENT:
					printF( "VK_ERROR_EXTENSION_NOT_PRESENT \n" );
					break;
				case VK_ERROR_FEATURE_NOT_PRESENT:
					printF( "VK_ERROR_FEATURE_NOT_PRESENT \n" );
					break;
				case VK_ERROR_INCOMPATIBLE_DRIVER:
					printF( "VK_ERROR_INCOMPATIBLE_DRIVER \n" );
					break;
				case VK_ERROR_TOO_MANY_OBJECTS:
					printF( "VK_ERROR_TOO_MANY_OBJECTS \n" );
					break;
				case VK_ERROR_FORMAT_NOT_SUPPORTED:
					printF( "VK_ERROR_FORMAT_NOT_SUPPORTED \n" );
					break;
				case VK_ERROR_FRAGMENTED_POOL:
					printF( "VK_ERROR_FRAGMENTED_POOL \n" );
					break;
				case VK_ERROR_SURFACE_LOST_KHR:
					printF( "VK_ERROR_SURFACE_LOST_KHR \n" );
					break;
				case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
					printF( "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR \n" );
					break;
				case VK_SUBOPTIMAL_KHR:
					printF( "VK_SUBOPTIMAL_KHR \n" );
					break;
				case VK_ERROR_OUT_OF_DATE_KHR:
					printF( "VK_ERROR_OUT_OF_DATE_KHR \n" );
					break;
				case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
					printF( "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR \n" );
					break;
				case VK_ERROR_VALIDATION_FAILED_EXT:
					printF( "VK_ERROR_VALIDATION_FAILED_EXT \n" );
					break;
				default:
					printF( "SOMETHING WRONG WITH VK \n" );
					break;
				}

				throwEx("Vulkan runtime ERROR!!");
			}
#endif
		}

	private:
		DebugLog();
		~DebugLog();
	};

}

#endif // DEBUG_LOG_H
