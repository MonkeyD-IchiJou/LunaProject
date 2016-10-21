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

	void DescriptorTool::SetUpDescriptorLayout(const VkDevice& logicaldevice, const uint32_t& numdescriptors, const VulkanDescriptorLayoutInfo* descriptors)
	{
		// destroy the previous layout if have any
		if (descriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(logicaldevice, descriptorSetLayout, nullptr);
			descriptorSetLayout = VK_NULL_HANDLE;
		}

		// set up the layout for the shaders to have a better understanding of what i am going to send to it
		std::vector<VkDescriptorSetLayoutBinding> layoutbindings(numdescriptors);
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

	void DescriptorTool::SetUpDescriptorPools(const VkDevice& logicaldevice, const uint32_t& numpoolsize, const VkDescriptorPoolSize* poolsizes, const uint32_t& maxsets)
	{
		// destroy the previous descriptor pool if have any
		if (descriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(logicaldevice, descriptorPool, nullptr);
			descriptorPool = VK_NULL_HANDLE;
			descriptorSets.clear();
		}

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = numpoolsize;
		poolInfo.pPoolSizes = poolsizes;
		poolInfo.maxSets = maxsets;
		DebugLog::EC(vkCreateDescriptorPool(logicaldevice, &poolInfo, nullptr, &descriptorPool));

		// prepare the descriptor sets
		descriptorSets.resize(maxsets);
	}

	void DescriptorTool::AddDescriptorSet(const VkDevice & logicaldevice, const int& whichset)
	{
		// then allocate the descriptor set
		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = descriptorPool;
		allocinfo.descriptorSetCount = 1;
		allocinfo.pSetLayouts = &descriptorSetLayout;
		vkAllocateDescriptorSets(logicaldevice, &allocinfo, &descriptorSets[whichset]);
	}

	void DescriptorTool::UpdateDescriptorSets(const VkDevice& logicaldevice, const int &whichset, const int &totalbinding, const VulkanDescriptorSetInfo* descriptorsetsinfo)
	{
		std::vector<VkWriteDescriptorSet> descriptorWrites(totalbinding);
		for (int i = 0; i < descriptorWrites.size(); ++i)
		{
			auto& descriptorwrite = descriptorWrites[i];
			auto& descriptor = descriptorsetsinfo[i];

			descriptorwrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorwrite.dstSet = descriptorSets[whichset];
			descriptorwrite.dstBinding = descriptor.layoutinfo.binding;
			descriptorwrite.dstArrayElement = 0;
			descriptorwrite.descriptorType = descriptor.layoutinfo.type;
			descriptorwrite.descriptorCount = 1;

			// if is buffer type
			if (descriptor.layoutinfo.typeflags == 0)
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

	void DescriptorTool::UpdateDescriptorSets(const VkDevice & logicaldevice, const int &whichset, const VulkanDescriptorSetInfo& descriptorsetsinfo)
	{
		VkWriteDescriptorSet descriptorwrite = {};

		descriptorwrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorwrite.dstSet = descriptorSets[whichset];
		descriptorwrite.dstBinding = descriptorsetsinfo.layoutinfo.binding;
		descriptorwrite.dstArrayElement = 0;
		descriptorwrite.descriptorType = descriptorsetsinfo.layoutinfo.type;
		descriptorwrite.descriptorCount = 1;

		// if is buffer type
		if (descriptorsetsinfo.layoutinfo.typeflags == 0)
		{
			descriptorwrite.pBufferInfo = &descriptorsetsinfo.bufferinfo;
			descriptorwrite.pImageInfo = nullptr;
		}
		else
		{
			descriptorwrite.pBufferInfo = nullptr;
			descriptorwrite.pImageInfo = &descriptorsetsinfo.imageinfo;
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
			descriptorSets.clear();
		}
	}
}