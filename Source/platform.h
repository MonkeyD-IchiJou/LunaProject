#ifndef PLATFORM_H
#define PLATFORM_H

// WINDOWS
#if defined(_WIN32)

#define VK_USE_PLATFORM_WIN32_KHR			1
#define PLATFORM_SURFACE_EXTENSION_NAME		VK_KHR_WIN32_SURFACE_EXTENSION_NAME

#include <Windows.h>
#include <vulkan\vulkan.h>

// ANDROID
#elif defined(__ANDROID__)

#define VK_USE_PLATFORM_ANDROID_KHR			1
#define PLATFORM_SURFACE_EXTENSION_NAME		VK_KHR_ANDROID_SURFACE_EXTENSION_NAME

#include <android_native_app_glue.h>
#include "vulkan_wrapper.h"

#else
#error Platform not yet supported
#endif

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
