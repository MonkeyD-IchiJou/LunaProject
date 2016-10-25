#ifndef TEXTURE_RESOURCES_H
#define TEXTURE_RESOURCES_H

#include "platform.h"
#include <vector>
#include <mutex>

namespace luna
{
	class VulkanImageBufferObject;
	class Font;

	class TextureResources
	{
	public:
		std::vector<VulkanImageBufferObject*> Textures;
		std::vector<Font*> Fonts;

	public:
		/* Singleton class implementation */
		static inline TextureResources* getInstance(void)
		{
			// only called once
			std::call_once(m_sflag, [&]() {
				m_instance = new TextureResources();
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

		TextureResources();
		~TextureResources() {};

	private:
		VkDevice m_logicaldevice = VK_NULL_HANDLE;

		static std::once_flag m_sflag;
		static TextureResources* m_instance;
	};
}

#endif

