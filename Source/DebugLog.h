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
		/* print what message is it on the console with endl*/
		template<typename t>
		static void printL(const t& format)
		{
#if _DEBUG
			std::cout << format << std::endl;
#endif //BUILD_ENABLE_DEBUG
		}

		/* print what message is it on the console without endl */
		template<typename t>
		static void print(const t& format)
		{
#if _DEBUG
			std::cout << format;
#endif //BUILD_ENABLE_DEBUG
		}

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
					printL( "VK_ERROR_OUT_OF_HOST_MEMORY" );
					break;
				case VK_ERROR_OUT_OF_DEVICE_MEMORY:
					printL( "VK_ERROR_OUT_OF_DEVICE_MEMORY" );
					break;
				case VK_ERROR_INITIALIZATION_FAILED:
					printL( "VK_ERROR_INITIALIZATION_FAILED" );
					break;
				case VK_ERROR_DEVICE_LOST:
					printL( "VK_ERROR_DEVICE_LOST" );
					break;
				case VK_ERROR_MEMORY_MAP_FAILED:
					printL( "VK_ERROR_MEMORY_MAP_FAILED" );
					break;
				case VK_ERROR_LAYER_NOT_PRESENT:
					printL( "VK_ERROR_LAYER_NOT_PRESENT" );
					break;
				case VK_ERROR_EXTENSION_NOT_PRESENT:
					printL( "VK_ERROR_EXTENSION_NOT_PRESENT" );
					break;
				case VK_ERROR_FEATURE_NOT_PRESENT:
					printL( "VK_ERROR_FEATURE_NOT_PRESENT" );
					break;
				case VK_ERROR_INCOMPATIBLE_DRIVER:
					printL( "VK_ERROR_INCOMPATIBLE_DRIVER" );
					break;
				case VK_ERROR_TOO_MANY_OBJECTS:
					printL( "VK_ERROR_TOO_MANY_OBJECTS" );
					break;
				case VK_ERROR_FORMAT_NOT_SUPPORTED:
					printL( "VK_ERROR_FORMAT_NOT_SUPPORTED" );
					break;
				case VK_ERROR_FRAGMENTED_POOL:
					printL( "VK_ERROR_FRAGMENTED_POOL" );
					break;
				case VK_ERROR_SURFACE_LOST_KHR:
					printL( "VK_ERROR_SURFACE_LOST_KHR" );
					break;
				case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
					printL( "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR" );
					break;
				case VK_SUBOPTIMAL_KHR:
					printL( "VK_SUBOPTIMAL_KHR" );
					break;
				case VK_ERROR_OUT_OF_DATE_KHR:
					printL( "VK_ERROR_OUT_OF_DATE_KHR" );
					break;
				case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
					printL( "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR" );
					break;
				case VK_ERROR_VALIDATION_FAILED_EXT:
					printL( "VK_ERROR_VALIDATION_FAILED_EXT" );
					break;
				default:
					printL( "SOMETHING WRONG WITH VK" );
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
