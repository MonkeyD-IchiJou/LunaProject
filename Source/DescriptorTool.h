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

	class DescriptorTool
	{
	public:
		DescriptorTool();
		~DescriptorTool();

		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> descriptorSets;

		void SetUpDescriptorLayout(const VkDevice& logicaldevice, const uint32_t& numdescriptors, const VulkanDescriptorLayoutInfo* descriptors);
		void SetUpDescriptorPools(const VkDevice& logicaldevice, const uint32_t& numpoolsize,const VkDescriptorPoolSize* poolsizes, const uint32_t& maxsets);
		void AddDescriptorSet(const VkDevice& logicaldevice, const int& whichset);

		// update all the descriptors by writing to the descriptor set
		void UpdateDescriptorSets(const VkDevice& logicaldevice, const int &whichset, const int &totalbinding, const VulkanDescriptorSetInfo* descriptorsetsinfo);

		// update the descriptor set based on index
		void UpdateDescriptorSets(const VkDevice& logicaldevice, const int &whichset, const VulkanDescriptorSetInfo& descriptorsetsinfo);

		void Destroy(const VkDevice& logicaldevice);
	};
}

#endif