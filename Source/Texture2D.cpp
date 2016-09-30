#include "Texture2D.h"
#include "BasicImage.h"
#include "BasicImageAttachment.h"
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
		BasicImage* image = dynamic_cast<BasicImage*>(m_image2D);
		image->SetImageSampler(sampler);
	}

	Texture2D::Texture2D(const eATTACHMENT_CREATE_TYPE & type, const uint32_t& texwidth, const uint32_t& texheight, const VkImageAspectFlags & aspectFlags)
	{
		m_image2D = new BasicImageAttachment(texwidth, texheight, aspectFlags);
		auto* image = dynamic_cast<BasicImageAttachment*>(m_image2D);

		switch (type)
		{
		case DEPTH_32_ATTACHMENT:
			// create the image buffer for it
			image->CreateImageBuffer(VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);

			break;

		default:
			DebugLog::throwEx("no such attachment");
			break;
		}
	}

	Texture2D::~Texture2D()
	{
		if (m_image2D != nullptr)
		{
			delete m_image2D;
		}
	}
}