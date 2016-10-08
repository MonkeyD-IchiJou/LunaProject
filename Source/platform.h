#ifndef PLATFORM_H
#define PLATFORM_H

// WINDOWS
#if defined(_WIN32)

#define VK_USE_PLATFORM_WIN32_KHR			1
#define PLATFORM_SURFACE_EXTENSION_NAME		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
#include <Windows.h>

// LINUX
#elif defined(__linux)

#define VK_USE_PLATFORM_XCB_KHR				1
#define PLATFORM_SURFACE_EXTENSION_NAME		VK_KHR_XCB_SURFACE_EXTENSION_NAME
#include <xcb/xcb.h>

#else 
#error Platform not yet supported
#endif 

// include the vulkan header 
#include <vulkan\vulkan.h>

#include <string>

inline const std::string getAssetPath()
{
#if defined(__ANDROID__)
	return "";
#else
	return "./../Assets/";
#endif
}

#endif //PLATFORM_H
