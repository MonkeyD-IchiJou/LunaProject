# Vulkan Game Dev README

A cross-platform game engine created with Vulkan Api and c++.
For windows and android only

# Building
all the source codes, external libraries and mk files are in the repo.

# Visual Studio
In order to build in debug mode, you may need to download and install the latest VulkanSDK from https://vulkan.lunarg.com/ 
I am using visual studio community 2015

# Android Studio
Android studio 2.2 or above is required for android builds.
latest Ndk is required, download it from sdk (if dont have it)
Build only support arm-v7
Important: You need to have a device with an Android image that supports Vulkan

# Current engine Features
- Multithreaded environment
I tried to make the whole framework as multithreaded-friendly as possible. Current game looping models 
-----------------------------------------------------------------------
Update Frame 0 | Update Frame 1 | Update Frame 2 | ...
-----------------------------------------------------------------------
~~~~~~~~~~~~~~ | Render Frame 0 | Render Frame 1 | Render Frame 2
-----------------------------------------------------------------------
