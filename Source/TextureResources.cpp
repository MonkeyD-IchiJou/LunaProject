#include "TextureResources.h"
#include "VulkanTexture2D.h"
#include "VulkanTextureArray2D.h"
#include "VulkanTextureCube.h"
#include "Renderer.h"

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

		Textures[BASIC_2D_RGBA8] = new VulkanTexture2D(getAssetPath() + "Textures/NewTex_rgba.ktx");
		Textures[BASIC_2D_BC2] = new VulkanTexture2D(getAssetPath() + "Textures/pattern_02_bc2.ktx");
		Textures[EVAFONT_2D_BC3] = new VulkanTexture2D(getAssetPath() + "Textures/eva_bc3.ktx");
		Textures[TERRAIN_2DARRAY_BC3] = new VulkanTextureArray2D(getAssetPath() + "Textures/terrain_texturearray_bc3.ktx");
		Textures[BASIC_2DARRAY_BC3] = new VulkanTextureArray2D(getAssetPath() + "Textures/texturearray_bc3.ktx");
		Textures[YOKOHOMO_CUBEMAP_BC3] = new VulkanTextureCube(getAssetPath() + "Textures/cubemap_yokohama.ktx");
		
		Textures[WORLDPOS_ATTACHMENT_RGBA16F] = nullptr;
		Textures[WORLDNORM_ATTACHMENT_RGBA16F] = nullptr;
		Textures[ALBEDO_ATTACHMENT_RGBA16F] = nullptr;
		Textures[HDRTEX_ATTACHMENT_RGBA16F] = nullptr;
		Textures[DEPTH_ATTACHMENT_32F] = nullptr;

		Textures[COMPUTETARGET_2D_RGBA16F] = new VulkanTexture2D(
			1280, 720, 
			VK_FORMAT_R16G16B16A16_SFLOAT,
			VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT, // sampled in the frag shader and used as storage target in the comp shader
			VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL
		);
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
