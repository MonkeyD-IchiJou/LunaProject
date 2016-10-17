#include "ComputeShader.h"
#include "VulkanImageBufferObject.h"
#include "DebugLog.h"

namespace luna
{
	ComputeShader::ComputeShader()
	{
	}

	ComputeShader::~ComputeShader()
	{
		Destroy();
	}

	void ComputeShader::Init(const VkRenderPass & renderpass)
	{
		// only when it has not created
		if (m_Pipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo computeinfo = CreateShaders_(getAssetPath() + "Shaders/compute_comp.spv");
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

	void ComputeShader::Bind(const VkCommandBuffer & commandbuffer)
	{
		// Compute Pipeline Binding (shaders binding)
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_Pipeline);

		// bind the descriptor sets using 
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_COMPUTE, m_pipelineLayout, 0, 1, &m_descriptorTool.descriptorSets, 0, nullptr);
	}

	void ComputeShader::SetDescriptors(const VulkanImageBufferObject* readimage, const VulkanImageBufferObject* writeimage)
	{
		m_descriptorTool.Destroy(m_logicaldevice);

		VkDescriptorImageInfo readimageinfo{};
		readimageinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // compute shader read image
		readimageinfo.imageView = readimage->getImageView();
		readimageinfo.sampler = readimage->getSampler();

		VkDescriptorImageInfo writeimageinfo{};
		writeimageinfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL; // compute shader write image
		writeimageinfo.imageView = writeimage->getImageView();
		writeimageinfo.sampler = writeimage->getSampler();

		// 2 descriptors to send to
		m_descriptorTool.descriptors.resize(2);

		auto& readimagedescriptor = m_descriptorTool.descriptors[0];
		readimagedescriptor.binding = 0;
		readimagedescriptor.imageinfo = readimageinfo;
		readimagedescriptor.shaderstage = VK_SHADER_STAGE_COMPUTE_BIT;
		readimagedescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		readimagedescriptor.typeflags = 1; // an image

		auto& writeimagedescriptor = m_descriptorTool.descriptors[1];
		writeimagedescriptor.binding = 1;
		writeimagedescriptor.imageinfo = writeimageinfo;
		writeimagedescriptor.shaderstage = VK_SHADER_STAGE_COMPUTE_BIT;
		writeimagedescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		writeimagedescriptor.typeflags = 1; // an image

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice);
		m_descriptorTool.SetUpDescriptorSets(m_logicaldevice);
		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice);
	}

	void ComputeShader::CreatePipelineLayout_()
	{
		if (m_descriptorTool.descriptorSetLayout == VK_NULL_HANDLE)
		{
			DebugLog::throwEx("descriptor set layout not init");
		}

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorTool.descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		DebugLog::EC(vkCreatePipelineLayout(m_logicaldevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
	}

	void ComputeShader::Destroy()
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