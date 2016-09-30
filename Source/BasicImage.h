#ifndef BASIC_IMAGE_H
#define BASIC_IMAGE_H

#include "VulkanImageBufferObject.h"

namespace luna
{
	class BasicImage : public VulkanImageBufferObject
	{
	public:
		BasicImage(unsigned char* pixels, const int& texwidth, const int& texheight, const int& texchannels);
		virtual ~BasicImage();

		/* map the pixels to the device memory */
		const void MapToDeviceMemory(const VkDeviceMemory& devicememory);

		/* image layout changes for both buffer */
		void TransitionStagedAndMainImageLayout();

		/* copy the staged image buffer into the main image buffer*/
		void CopyImage(const VkCommandBuffer& commandbuffer);

		/* create the image view */
		void CreateImageView(const VkFormat& format, const VkImageAspectFlags& aspectFlags) override;

		/* get stage buffer data */
		inline const auto& GetStageBufferData() const { return m_stagebuff; }

		/* get the image sampler for this image */
		inline const auto GetImageSampler() const { return m_imagesampler; }

		/* get the staged buffer offset in the main device memory */
		inline const auto GetStagedBufferOffset() const { return m_stagedBufferOffset; }

		/* set the image buffer offset in the device memory */
		inline void SetStagedBufferOffset(VkDeviceSize stagedBufferOffset) { this->m_stagedBufferOffset = stagedBufferOffset; }

		/* set the image sampler for this image */
		inline void SetImageSampler(VkSampler sampler) { this->m_imagesampler = sampler; }

	private:
		/* the pixels in this image */
		unsigned char* m_pixels = nullptr;

		/* the total channels in this channel */
		int m_texchannels = 0;

		/* the beginning offset (aligned bytes) of the image buffer to be mapped of, in the device memory */
		VkDeviceSize m_stagedBufferOffset = 0;

		/* the staging image buff for uploading image pixels data*/
		VulkanImageBufferData m_stagebuff{};

		/* handle image sampler */
		VkSampler m_imagesampler = VK_NULL_HANDLE;
	};
}

#endif

