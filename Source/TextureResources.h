#ifndef TEXTURE_RESOURCES_H
#define TEXTURE_RESOURCES_H

#include "platform.h"
#include <vector>
#include <mutex>

namespace luna
{
	class VulkanImageBufferObject;

	enum eTEXTURES
	{
		BASIC_2D_RGBA8 = 0,
		BASIC_2D_BC2,
		TERRAIN_2DARRAY_BC3,
		BASIC_2DARRAY_BC3,
		YOKOHOMO_CUBEMAP_BC3,
		WORLDPOS_2D_RGBA16FLOAT,
		WORLDNORMAL_2D_RGBA16FLOAT,
		ALBEDO_2D_RGBA8UNORM,
		DEPTH_2D_32FLOAT,
		LIGHTPROCESS_2D_RGBA8UNORM, // color output from light pass
		MAXTEX_NAME_TYPE_FORMAT
	};

	class TextureResources
	{
	public:
		std::vector<VulkanImageBufferObject*> Textures;

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

