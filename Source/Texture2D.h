#ifndef TEXTURE_2D_H
#define TEXTURE_2D_H

#include "platform.h"
#include <string>

namespace luna
{
	class BasicImage;

	class Texture2D
	{
	public:
		Texture2D(const std::string & path, const VkSampler& sampler);
		~Texture2D();

		BasicImage* getImage() const { return m_image2D; }

	private:
		BasicImage* m_image2D = nullptr;
	};
}

#endif

