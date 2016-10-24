#ifndef DESCRIPTOR_TOOL_H
#define DESCRIPTOR_TOOL_H

#include "platform.h"
#include <vector>

namespace luna
{
	struct VulkanDescriptorLayoutInfo
	{
		VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		uint32_t binding = 0;
		VkShaderStageFlags shaderstage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		uint32_t typeflags = 0; // 0 == buffertype, 1 == imagetype, 2 == texelbuffertype
	};

	struct VulkanDescriptorSetInfo
	{
		VulkanDescriptorLayoutInfo layoutinfo{};
		VkDescriptorBufferInfo bufferinfo{};
		VkDescriptorImageInfo imageinfo{};
	};

	struct DescriptorsInfo
	{
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> descriptorSets;

		void Destroy(VkDevice logicaldevice)
		{
			if (descriptorSetLayout != VK_NULL_HANDLE)
			{
				vkDestroyDescriptorSetLayout(logicaldevice, descriptorSetLayout, nullptr);
				descriptorSetLayout = VK_NULL_HANDLE;

				descriptorSets.clear();
			}
		}
	};

	class DescriptorTool
	{
	public:
		DescriptorTool();
		~DescriptorTool();

		// i can have different kind of descriptorSetLayouts, and each descriptorSetLayouts, i can have different kind of descriptor set too
		std::vector<DescriptorsInfo> descriptorsInfo;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;

		void SetUpDescriptorLayout(const VkDevice& logicaldevice, const uint32_t& numbindings, const VulkanDescriptorLayoutInfo* descriptors);
		void SetUpDescriptorPools(const VkDevice& logicaldevice, const uint32_t& numpoolsize,const VkDescriptorPoolSize* poolsizes, const uint32_t& maxsets);
		void AddDescriptorSet(const VkDevice& logicaldevice, const int& whichlayoutset, const int& whichset);

		// update all the descriptors by writing to the descriptor set
		void UpdateDescriptorSets(const VkDevice& logicaldevice, const int& whichlayoutset, const int &whichset, const int &totalbinding, const VulkanDescriptorSetInfo* descriptorsetsinfo);

		// update the descriptor set based on index
		void UpdateDescriptorSets(const VkDevice& logicaldevice, const int& whichlayoutset, const int &whichset, const VulkanDescriptorSetInfo& descriptorsetsinfo);

		void Destroy(const VkDevice& logicaldevice);
	};
}

#endif