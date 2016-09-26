#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "VulkanBufferData.h"
#include <vector>
#include <mutex>

namespace luna
{
	class Model;
	class BasicUBO;

	enum eMODELS
	{
		QUAD_MODEL = 0,
		BOXES_MODEL = 1,
		BUNNY_MODEL = 2,
		TYRA_MODEL = 3,
		MAX_MODELS
	};

	/* all the mesh and texture resources are here */
	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();

		/* all the basic mesh resources is here */
		std::vector<Model*> Models;
		BasicUBO* UBO;

	public:
		/* Singleton class implementation */
		static inline ResourceManager* getInstance(void)
		{
			// only called once
			std::call_once(m_sflag, [&]() {
				m_instance = new ResourceManager();
			});

			return m_instance;
		}

		/* check whether exist or not */
		static inline bool exists(void)
		{
			return m_instance != nullptr;
		}

		/* Warning Once destroyed, forever destroy */
		void Destroy();

	private:
		void Init_();
		void LoadToDevice_();
		void StagingBufferInit_();
		void DeviceBufferInit_();
		void CopyBufferToDevice_();

	private:
		VkDevice m_logicaldevice = VK_NULL_HANDLE;

		VulkanBufferData m_stagingBuffer{};
		VkDeviceMemory m_devicememory_staged = VK_NULL_HANDLE;
		VulkanBufferData m_mainBuffer{};
		VkDeviceMemory m_devicememory_main = VK_NULL_HANDLE;

		static std::once_flag m_sflag;
		static ResourceManager* m_instance;
	};

}

#endif
