#include "TextureResources.h"
#include "VulkanTexture2D.h"
#include "VulkanTextureArray2D.h"
#include "VulkanTextureCube.h"
#include "Renderer.h"
#include "enum_c.h"
#include "Font.h"

namespace luna
{
	std::once_flag TextureResources::m_sflag{};
	TextureResources* TextureResources::m_instance = nullptr;

	TextureResources::TextureResources()
	{
		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();

		// init all the textures
		Init_();
	}

	void TextureResources::Init_()
	{
		Textures.resize(MAXTEX_NAME_TYPE_FORMAT);

		Textures[BASIC_2D_RGBA8] = new VulkanTexture2D(getAssetPath() + "Textures/pattern_02_rgba8.ktx");
		Textures[BLACK_2D_RGBA] = new VulkanTexture2D(getAssetPath() + "Textures/black.ktx");
		Textures[EVAFONT_2D_BC3] = new VulkanTexture2D(getAssetPath() + "Textures/eva_rgba8.ktx");
		Textures[YOKOHOMO_CUBEMAP_RGBA8] = new VulkanTextureCube(getAssetPath() + "Textures/cubemap_vulkan.ktx");
		
		/* attachments images will be auto init by fbos */
		Textures[COLOR0_ATTACHMENT_RGBA32U] = nullptr;
		Textures[COLOR1_ATTACHMENT_RGBA32U] = nullptr;
		Textures[COLOR2_ATTACHMENT_RGBA32F] = nullptr;
		Textures[LIGHTINGTEX_ATTACHMENT_RGBA16F] = nullptr;
		Textures[NONLIGHTINGTEX_ATTACHMENT_RGBA8U] = nullptr;
		Textures[HDRTEX_ATTACHMENT_RGBA16F] = nullptr;
		Textures[DEPTHSTENCIL_ATTACHMENT_32F] = nullptr;

		Textures[HORBLUR_2D_RGBA16F] = new VulkanTexture2D(
			512, 512, 
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL
		);

		Textures[VERTBLUR_2D_RGBA16F] = new VulkanTexture2D(
			512, 512, 
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_STORAGE_BIT,
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL
		);

		// font resource init
		Fonts.resize(MAX_FONT);

		Fonts[FONT_EVA] = new Font();
		Fonts[FONT_EVA]->LoadFonts(
			getAssetPath() + "Fonts/eva.fnt", 
			(float)Textures[EVAFONT_2D_BC3]->getWidth(), 
			(float)Textures[EVAFONT_2D_BC3]->getHeight()
		);
	}

	void TextureResources::Destroy()
	{
		for (auto& i : Textures)
		{
			delete i;
		}
		Textures.clear();

		for (auto& i : Fonts)
		{
			delete i;
		}
		Fonts.clear();
	}
}
