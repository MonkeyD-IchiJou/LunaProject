#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H

#include "platform.h"
#include <string>

namespace luna
{
	class VulkanImageBufferObject;

	enum eATTACHMENT_CREATE_TYPE
	{
		DEPTH_32_ATTACHMENT = 0,
		MAX_ATTACHMENTS_CREATE_TYPE
	};

	class Texture2D
	{
	public:
		Texture2D(const std::string & path, const VkSampler& sampler);
		Texture2D(const eATTACHMENT_CREATE_TYPE& type, const uint32_t& texwidth, const uint32_t& texheight, const VkImageAspectFlags & aspectFlags);
		~Texture2D();

		template<typename T>
		auto getImage() const { return dynamic_cast<T>(m_image2D); };

	private:
		VulkanImageBufferObject* m_image2D = nullptr;
	};
}

#endif

