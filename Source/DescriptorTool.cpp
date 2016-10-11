#include "DescriptorTool.h"
#include "DebugLog.h"

namespace luna
{
	DescriptorTool::DescriptorTool()
	{
	}

	DescriptorTool::~DescriptorTool()
	{
	}

	void DescriptorTool::SetUpDescriptorLayout(const VkDevice& logicaldevice)
	{
		// destroy the previous layout if have any
		if (descriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(logicaldevice, descriptorSetLayout, nullptr);
			descriptorSetLayout = VK_NULL_HANDLE;
		}

		// set up the layout for the shaders to have a better understanding of what i am going to send to it
		std::vector<VkDescriptorSetLayoutBinding> layoutbindings(descriptors.size());
		for (int i = 0; i < layoutbindings.size(); ++i)
		{
			layoutbindings[i].binding = descriptors[i].binding;
			layoutbindings[i].descriptorType = descriptors[i].type;
			layoutbindings[i].descriptorCount = 1;
			layoutbindings[i].stageFlags = descriptors[i].shaderstage;
			layoutbindings[i].pImmutableSamplers = nullptr; // related to image sampling
		}

		VkDescriptorSetLayoutCreateInfo descriptorSetLayout_createinfo{};
		descriptorSetLayout_createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayout_createinfo.bindingCount = static_cast<uint32_t>(layoutbindings.size());
		descriptorSetLayout_createinfo.pBindings = layoutbindings.data();
		DebugLog::EC(vkCreateDescriptorSetLayout(logicaldevice, &descriptorSetLayout_createinfo, nullptr, &descriptorSetLayout));
	}

	void DescriptorTool::SetUpDescriptorSets(const VkDevice& logicaldevice)
	{
		// destroy the previous descriptor pool if have any
		if (descriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(logicaldevice, descriptorPool, nullptr);
			descriptorPool = VK_NULL_HANDLE;
			descriptorSets = VK_NULL_HANDLE;
		}

		// create the descriptor pool first, based on how many descriptors i have
		std::vector<VkDescriptorPoolSize> poolSizes(descriptors.size());
		for (int i = 0; i < poolSizes.size(); ++i)
		{
			poolSizes[i].type = descriptors[i].type;
			poolSizes[i].descriptorCount = 1;
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1;
		DebugLog::EC(vkCreateDescriptorPool(logicaldevice, &poolInfo, nullptr, &descriptorPool));

		// then allocate the descriptor set
		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = descriptorPool;
		allocinfo.descriptorSetCount = 1;
		allocinfo.pSetLayouts = &descriptorSetLayout;
		vkAllocateDescriptorSets(logicaldevice, &allocinfo, &descriptorSets);
	}

	void DescriptorTool::UpdateDescriptorSets(const VkDevice& logicaldevice)
	{
		// update the descriptor set, based on how many descriptors i have
		std::vector<VkWriteDescriptorSet> descriptorWrites(descriptors.size());
		for (int i = 0; i < descriptorWrites.size(); ++i)
		{
			auto& descriptorwrite = descriptorWrites[i];
			auto& descriptor = descriptors[i];

			descriptorwrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorwrite.dstSet = descriptorSets;
			descriptorwrite.dstBinding = descriptor.binding;
			descriptorwrite.dstArrayElement = 0;
			descriptorwrite.descriptorType = descriptor.type;
			descriptorwrite.descriptorCount = 1;

			// if is buffer type
			if (descriptor.typeflags == 0)
			{
				descriptorwrite.pBufferInfo = &descriptor.bufferinfo;
				descriptorwrite.pImageInfo = nullptr;
			}
			else
			{
				descriptorwrite.pBufferInfo = nullptr;
				descriptorwrite.pImageInfo = &descriptor.imageinfo;
			}
		}

		vkUpdateDescriptorSets(logicaldevice, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void DescriptorTool::UpdateDescriptorSets(const VkDevice & logicaldevice, const int & i)
	{
		VkWriteDescriptorSet descriptorwrite = {};
		auto& descriptor = descriptors[i];

		descriptorwrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorwrite.dstSet = descriptorSets;
		descriptorwrite.dstBinding = descriptor.binding;
		descriptorwrite.dstArrayElement = 0;
		descriptorwrite.descriptorType = descriptor.type;
		descriptorwrite.descriptorCount = 1;

		// if is buffer type
		if (descriptor.typeflags == 0)
		{
			descriptorwrite.pBufferInfo = &descriptor.bufferinfo;
			descriptorwrite.pImageInfo = nullptr;
		}
		else
		{
			descriptorwrite.pBufferInfo = nullptr;
			descriptorwrite.pImageInfo = &descriptor.imageinfo;
		}

		vkUpdateDescriptorSets(logicaldevice, 1, &descriptorwrite, 0, nullptr);
	}

	void DescriptorTool::Destroy(const VkDevice & logicaldevice)
	{
		if (descriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(logicaldevice, descriptorSetLayout, nullptr);
			descriptorSetLayout = VK_NULL_HANDLE;
		}

		if (descriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(logicaldevice, descriptorPool, nullptr);
			descriptorPool = VK_NULL_HANDLE;
			descriptorSets = VK_NULL_HANDLE;
		}

		descriptors.clear();
	}
}