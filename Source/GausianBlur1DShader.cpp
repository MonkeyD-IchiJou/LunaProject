#include "GausianBlur1DShader.h"
#include "VulkanImageBufferObject.h"
#include "DebugLog.h"
#include <array>

namespace luna
{
	GausianBlur1DShader::GausianBlur1DShader()
	{
	}

	GausianBlur1DShader::~GausianBlur1DShader()
	{
		Destroy();
	}

	void GausianBlur1DShader::Init(const VkRenderPass & renderpass, uint32_t subpassindex)
	{
		// only when it has not created
		if (m_Pipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo computeinfo = CreateShaders_(getAssetPath() + "Shaders/gausianblur1d_comp.spv");
			computeinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
			computeinfo.pName = "main";

			/* set up the fix pipeline info */
			FixedPipelineCreationTool fixedpipelineinfo{};
			SetUpFixedPipeline_(fixedpipelineinfo);

			/* set up the pipeline layout if have any */
			CreatePipelineLayout_();

			/* create the graphics pipeline */
			VkComputePipelineCreateInfo computepipeline_createinfo{};
			computepipeline_createinfo.sType					= VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
			computepipeline_createinfo.layout					= m_pipelineLayout;
			computepipeline_createinfo.stage					= computeinfo;
			computepipeline_createinfo.flags					= 0;
			computepipeline_createinfo.basePipelineHandle		= VK_NULL_HANDLE;
			computepipeline_createinfo.basePipelineIndex		= -1;

			DebugLog::EC(vkCreateComputePipelines(m_logicaldevice, VK_NULL_HANDLE, 1, &computepipeline_createinfo, nullptr, &m_Pipeline));

			/* destroy the shader modules as they are useless now */
			if (computeinfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, computeinfo.module, nullptr); }
		}
	}

	void GausianBlur1DShader::Bind(const VkCommandBuffer & commandbuffer)
	{
		// Compute Pipeline Binding (shaders binding)
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_Pipeline);
	}

	void GausianBlur1DShader::BindDescriptorSet(const VkCommandBuffer & commandbuffer, const int & whichset)
	{
		// bind the descriptor sets using 
		vkCmdBindDescriptorSets(
			commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 
			1, &m_descriptorTool.descriptorsInfo[0].descriptorSets[whichset], 
			0, nullptr
		);
	}

	void GausianBlur1DShader::SetDescriptors(const VulkanImageBufferObject* horizontalImage, const VulkanImageBufferObject* VerticalImage)
	{
		m_descriptorTool.Destroy(m_logicaldevice);

		// 2 kind of descriptors to send to
		// set up the layout for the shaders 
		const int totalbinding = 2;
		std::array<VulkanDescriptorLayoutInfo, totalbinding> layoutinfo{};
		
		// read image
		layoutinfo[0].binding = 0;
		layoutinfo[0].shaderstage = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutinfo[0].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		layoutinfo[0].typeflags = 1; // an image

		// write image
		layoutinfo[1].binding = 1;
		layoutinfo[1].shaderstage = VK_SHADER_STAGE_COMPUTE_BIT;
		layoutinfo[1].type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		layoutinfo[1].typeflags = 1; // an image

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice, totalbinding, layoutinfo.data());

		// create the poolsize to hold all my descriptors
		const int totaldescriptors = 4; // total num of descriptors
		const int totalsets = 2; // total num of descriptor sets i will have

		VkDescriptorPoolSize poolSize;
		poolSize.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		poolSize.descriptorCount = totaldescriptors; // all my descriptors are the same type

		m_descriptorTool.SetUpDescriptorPools(m_logicaldevice, 1, &poolSize, totalsets);
		m_descriptorTool.descriptorsInfo[0].descriptorSets.resize(2); // first layout has 2 descriptorsets
		m_descriptorTool.AddDescriptorSet(m_logicaldevice, 0, 0);
		m_descriptorTool.AddDescriptorSet(m_logicaldevice, 0, 1);

		// first descriptor set update
		VkDescriptorImageInfo readimageinfo{};
		readimageinfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // compute shader read image
		readimageinfo.imageView = horizontalImage->getImageView();
		readimageinfo.sampler = horizontalImage->getSampler();
		VkDescriptorImageInfo writeimageinfo{};
		writeimageinfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // compute shader write image
		writeimageinfo.imageView = VerticalImage->getImageView();
		writeimageinfo.sampler = VerticalImage->getSampler();

		std::array<VulkanDescriptorSetInfo, totalbinding> descriptorset{};
		descriptorset[0].layoutinfo = layoutinfo[0];
		descriptorset[0].imageinfo = readimageinfo;
		descriptorset[1].layoutinfo = layoutinfo[1];
		descriptorset[1].imageinfo = writeimageinfo;

		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 0, 0, totalbinding, descriptorset.data());

		// second descriptor set update
		readimageinfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // compute shader read image
		readimageinfo.imageView = VerticalImage->getImageView();
		readimageinfo.sampler = VerticalImage->getSampler();
		writeimageinfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // compute shader write image
		writeimageinfo.imageView = horizontalImage->getImageView();
		writeimageinfo.sampler = horizontalImage->getSampler();

		descriptorset[0].imageinfo = readimageinfo;
		descriptorset[1].imageinfo = writeimageinfo;

		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 0, 1, totalbinding, descriptorset.data());
	}

	void GausianBlur1DShader::LoadBlurDirection(const VkCommandBuffer & commandbuffer, const glm::ivec2 & blurdir)
	{
		vkCmdPushConstants(commandbuffer, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, 0, sizeof(blurdir), &blurdir);
	}

	void GausianBlur1DShader::LoadResolution(const VkCommandBuffer & commandbuffer, const glm::ivec2 & resolution)
	{
		vkCmdPushConstants(commandbuffer, m_pipelineLayout, VK_SHADER_STAGE_COMPUTE_BIT, sizeof(glm::ivec2), sizeof(resolution), &resolution);
	}

	void GausianBlur1DShader::CreatePipelineLayout_()
	{
		VkPushConstantRange pushconstant{};
		pushconstant.stageFlags = VK_SHADER_STAGE_COMPUTE_BIT;
		pushconstant.offset = 0;
		pushconstant.size = sizeof(glm::ivec2) + sizeof(glm::ivec2);

		std::vector<VkDescriptorSetLayout> setlayouts;
		setlayouts.resize(m_descriptorTool.descriptorsInfo.size());
		for (int i = 0; i < m_descriptorTool.descriptorsInfo.size(); ++i)
		{
			setlayouts[i] = m_descriptorTool.descriptorsInfo[i].descriptorSetLayout;
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setlayouts.size()); // how many descriptor layout i have
		pipelineLayoutInfo.pSetLayouts = setlayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushconstant;
		DebugLog::EC(vkCreatePipelineLayout(m_logicaldevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
	}

	void GausianBlur1DShader::Destroy()
	{
		m_descriptorTool.Destroy(m_logicaldevice);

		if (m_pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(m_logicaldevice, m_pipelineLayout, nullptr);
			m_pipelineLayout = VK_NULL_HANDLE;
		}

		if (m_Pipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_logicaldevice, m_Pipeline, nullptr);
			m_Pipeline = VK_NULL_HANDLE;
		}
	}
}