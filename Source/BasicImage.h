#ifndef BASIC_IMAGE_H
#define BASIC_IMAGE_H

#include "VulkanBufferData.h"

namespace luna
{
	class BasicImage
	{
	public:
		BasicImage(unsigned char* pixels, const int& texwidth, const int& texheight, const int& texchannels);
		~BasicImage();

		/* create the image buffer */
		void CreateImage();

		/* create the image view */
		void CreateImageView(const VkFormat& format, const VkImageAspectFlags& aspectFlags);

		/* copy the staged image buffer into the main image buffer*/
		void CopyImage(const VkCommandBuffer& commandbuffer);

		/* image layout changes for both buffer */
		void TransitionStagedAndMainImageLayout();

		/* map the pixels to the device memory */
		const void MapToDeviceMemory(const VkDeviceMemory& devicememory);

		inline const auto& GetStageBufferData() const { return m_stagebuff; }
		inline const auto& GetMainBufferData() const { return m_imagebuff; }
		inline const auto GetImageView() const { return m_imageview; }
		inline const auto GetImageSampler() const { return m_imagesampler; }
		inline const auto GetBufferOffset() const { return m_imageBufferOffset; }

		inline void SetImageSampler(VkSampler sampler) { this->m_imagesampler = sampler; }
		/* set the image buffer offset in the device memory */
		inline void SetImageBufferOffset(VkDeviceSize imageBufferOffset) { this->m_imageBufferOffset = imageBufferOffset; }

	private:
		/* image layout transitioning for better usage in gpu */
		void TransitionImageLayout_(VkImage srcimage, const VkImageLayout& oldLayout, const VkImageLayout& newLayout, const VkImageAspectFlags& aspectMask, 
			const VkAccessFlags& srcAccessMask, const VkAccessFlags& dstAccessMask);
		void CreateImageBuff_(VulkanImageBufferData& buff, const VkMemoryPropertyFlags & memoryproperties);
		static VkCommandBuffer BeginSingleTimeCommands_(const VkDevice & logicaldevice, VkCommandPool commandPool);
		static void EndSingleTimeCommands_(const VkDevice & logicaldevice, VkCommandBuffer commandBuffer);

	private:
		/* the pixels in this image */
		unsigned char* m_pixels = nullptr;

		/* the total width of this image */
		int m_texwidth = 0;

		/* the total height of this image */
		int m_texheight = 0;

		/* the total channels in this channel */
		int m_texchannels = 0;

		/* total image size in bytes */
		VkDeviceSize m_imageTotalSize = 0;

		/* the beginning offset (aligned bytes) of the image buffer to be mapped of*/
		VkDeviceSize m_imageBufferOffset = 0;

		/* the staging image buff for uploading image pixels data*/
		VulkanImageBufferData m_stagebuff{};

		/* the main image buffer to communicate with when drawing */
		VulkanImageBufferData m_imagebuff{};

		/* handle image view */
		VkImageView m_imageview = VK_NULL_HANDLE;

		/* handle image sampler */
		VkSampler m_imagesampler = VK_NULL_HANDLE;

		/* handle for the logical device */
		VkDevice m_logicaldevice = VK_NULL_HANDLE;
	};
}

#endif

