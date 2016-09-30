#include "BasicImage.h"
#include "DebugLog.h"
#include "Renderer.h"

#include <stb_image.h>

namespace luna
{

	BasicImage::BasicImage(unsigned char* pixels, const int& texwidth, const int& texheight, const int& texchannels) :
		m_pixels(pixels),
		m_texchannels(texchannels)
	{
		if (!m_pixels)
		{
			DebugLog::throwEx("invalid pixels image");
		}

		m_texwidth = texwidth;
		m_texheight = texheight;

		m_imageTotalSize = m_texwidth * m_texheight * 4; // rgba

		// create the staged image buff (datas will be mapped after resource manager create the stagedDeviceMemory)
		m_stagebuff.format = VK_FORMAT_R8G8B8A8_UNORM;
		m_stagebuff.tiling = VK_IMAGE_TILING_LINEAR;
		m_stagebuff.imageusage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		CreateImageBuff_(m_stagebuff, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		// create the main image buff 
		m_imagebuff.format = VK_FORMAT_R8G8B8A8_UNORM;
		m_imagebuff.tiling = VK_IMAGE_TILING_OPTIMAL;
		m_imagebuff.imageusage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		CreateImageBuff_(m_imagebuff, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	const void BasicImage::MapToDeviceMemory(const VkDeviceMemory & devicememory)
	{
		// need to make sure we have the pixels data before mapping
		if (m_pixels)
		{
			// map the data into staging image buffer
			void *data = nullptr;
			vkMapMemory(m_logicaldevice, devicememory, m_stagedBufferOffset, m_stagebuff.RequirementSizeInDeviceMem, 0, &data);
			memcpy(data, m_pixels, (size_t)m_stagebuff.RequirementSizeInDeviceMem);
			vkUnmapMemory(m_logicaldevice, devicememory);

			// can free the pixels once copied
			stbi_image_free(m_pixels);
			m_pixels = nullptr;
		}
	}

	void BasicImage::TransitionStagedAndMainImageLayout()
	{
		// need to transition the layout of both images before copying each other
		TransitionImageLayout_(m_stagebuff.buffer, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_READ_BIT);
		TransitionImageLayout_(m_imagebuff.buffer, VK_IMAGE_LAYOUT_PREINITIALIZED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_ACCESS_HOST_WRITE_BIT, VK_ACCESS_TRANSFER_WRITE_BIT);
	}

	void BasicImage::CopyImage(const VkCommandBuffer& commandbuffer)
	{
		VkImageSubresourceLayers subResource{};
		subResource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subResource.baseArrayLayer = 0;
		subResource.mipLevel = 0;
		subResource.layerCount = 1;

		VkImageCopy region{};
		region.srcSubresource = subResource;
		region.dstSubresource = subResource;
		region.srcOffset = { 0, 0, 0 };
		region.dstOffset = { 0, 0, 0 };
		region.extent.width = m_texwidth;
		region.extent.height = m_texheight;
		region.extent.depth = 1;

		vkCmdCopyImage(
			commandbuffer, 
			m_stagebuff.buffer, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
			m_imagebuff.buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			1, &region
		);
	}

	void BasicImage::CreateImageView(const VkFormat& format, const VkImageAspectFlags& aspectFlags)
	{
		// to be able to start sampling in the shader
		TransitionImageLayout_(m_imagebuff.buffer, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_ASPECT_COLOR_BIT,
			VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);

		// some clean up before create the image view
		if (m_stagebuff.buffer != VK_NULL_HANDLE)
		{
			vkDestroyImage(m_logicaldevice, m_stagebuff.buffer, nullptr);
			m_stagebuff.buffer = VK_NULL_HANDLE;
		}

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = m_imagebuff.buffer;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = format;
		viewInfo.subresourceRange.aspectMask = aspectFlags;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		DebugLog::EC(vkCreateImageView(m_logicaldevice, &viewInfo, nullptr, &m_imageview));
	}

	BasicImage::~BasicImage()
	{
		if (m_stagebuff.buffer != VK_NULL_HANDLE)
		{
			vkDestroyImage(m_logicaldevice, m_stagebuff.buffer, nullptr);
			m_stagebuff.buffer = VK_NULL_HANDLE;
		}

		if (m_pixels != nullptr)
		{
			stbi_image_free(m_pixels);
			m_pixels = nullptr;
		}
	}
}
