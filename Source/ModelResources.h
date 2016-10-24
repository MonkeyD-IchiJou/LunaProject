#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "VulkanBufferData.h"
#include <vector>
#include <mutex>

namespace luna
{
	class Model;

	/* all the mesh resources are here */
	class ModelResources
	{
	public:
		/* all the meshes is here */
		std::vector<Model*> Models;

	public:
		/* Singleton class implementation */
		static inline ModelResources* getInstance(void)
		{
			// only called once
			std::call_once(m_sflag, [&]() {
				m_instance = new ModelResources();
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
		void BufferInit_(VulkanBufferData& stagingBuffer);
		void StagingBufferBinding_(VulkanBufferData& stagingBuffer, VkDeviceMemory& devicememory_staged);
		void DeviceBufferInit_();
		void DeviceBufferBinding_();
		void CopyBufferToDevice_(VulkanBufferData& stagingBuffer);

		ModelResources();
		~ModelResources() {};

	private:
		VkDevice m_logicaldevice = VK_NULL_HANDLE;

		VulkanBufferData m_mainBuffer{};
		VkDeviceMemory m_devicememory_main = VK_NULL_HANDLE;

		static std::once_flag m_sflag;
		static ModelResources* m_instance;
	};

}

#endif
