#include "TextureResources.h"
#include "VulkanTexture2D.h"
#include "VulkanTextureArray2D.h"
#include "VulkanTextureCube.h"
#include "Renderer.h"
#include "WinNative.h"

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

		auto winwidth = WinNative::getInstance()->getWinSizeX();
		auto winheight = WinNative::getInstance()->getWinSizeY();

		Textures[BASIC_2D_RGBA8] = new VulkanTexture2D("./../Assets/Textures/font_sdf_rgba.ktx");
		Textures[BASIC_2D_BC2] = new VulkanTexture2D("./../Assets/Textures/pattern_02_bc2.ktx");
		Textures[TERRAIN_2DARRAY_BC3] = new VulkanTextureArray2D("./../Assets/Textures/terrain_texturearray_bc3.ktx");
		Textures[BASIC_2DARRAY_BC3] = new VulkanTextureArray2D("./../Assets/Textures/texturearray_bc3.ktx");
		Textures[YOKOHOMO_CUBEMAP_BC3] = new VulkanTextureCube("./../Assets/Textures/cubemap_yokohama.ktx");
		Textures[WORLDPOS_2D_RGBA16FLOAT] = nullptr;
		Textures[WORLDNORMAL_2D_RGBA16FLOAT] = nullptr;
		Textures[ALBEDO_2D_RGBA8UNORM] = nullptr;
		Textures[DEPTH_2D_32FLOAT] = nullptr;
		Textures[LIGHTPROCESS_2D_RGBA8UNORM] = nullptr;
	}

	void TextureResources::Destroy()
	{
		for (auto& i : Textures)
		{
			delete i;
		}
		Textures.clear();
	}
}
