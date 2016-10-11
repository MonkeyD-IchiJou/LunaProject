#ifndef DESCRIPTOR_TOOL_H
#define DESCRIPTOR_TOOL_H

#include "platform.h"
#include <vector>

namespace luna
{
	struct Descriptor
	{
		VkDescriptorType type = VK_DESCRIPTOR_TYPE_MAX_ENUM;
		VkDescriptorBufferInfo bufferinfo{};
		VkDescriptorImageInfo imageinfo{};
		uint32_t binding = 0;
		VkShaderStageFlags shaderstage = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
		uint32_t typeflags = 0; // 0 == buffertype, 1 == imagetype, 2 == texelbuffertype
	};

	class DescriptorTool
	{
	public:
		DescriptorTool();
		~DescriptorTool();

		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet descriptorSets = VK_NULL_HANDLE;
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;

		std::vector<Descriptor> descriptors;

		void SetUpDescriptorLayout(const VkDevice& logicaldevice);
		void SetUpDescriptorSets(const VkDevice& logicaldevice);

		// update all the descriptors by writing to the descriptor set
		void UpdateDescriptorSets(const VkDevice& logicaldevice);

		// update the descriptor set based on index
		void UpdateDescriptorSets(const VkDevice& logicaldevice, const int& i);

		void Destroy(const VkDevice& logicaldevice);
	};
}

#endif