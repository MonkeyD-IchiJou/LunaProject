#include "Texture2D.h"
#include "BasicImage.h"
#include "DebugLog.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace luna
{
	Texture2D::Texture2D(const std::string & path, const VkSampler& sampler)
	{
		int texWidth = 0, texHeight = 0, texChannels = 0;

		stbi_uc* pixels = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

		if (!pixels)
		{
			DebugLog::throwEx("failed to load a texture image");
		}

		// create the image and image view for it
		m_image2D = new BasicImage(pixels, texWidth, texHeight, texChannels);
		m_image2D->SetImageSampler(sampler);
	}

	Texture2D::~Texture2D()
	{
		if (m_image2D != nullptr)
		{
			delete m_image2D;
		}
	}
}