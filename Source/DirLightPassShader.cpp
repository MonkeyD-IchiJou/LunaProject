#include "DirLightPassShader.h"
#include "DebugLog.h"
#include "VulkanImageBufferObject.h"
#include "Vertex.h"

namespace luna
{
	DirLightPassShader::DirLightPassShader()
	{
	}

	DirLightPassShader::~DirLightPassShader()
	{
		Destroy();
	}

	void DirLightPassShader::Bind(const VkCommandBuffer & commandbuffer)
	{
		// Graphics Pipeline Binding (shaders binding)
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline);

		// bind the descriptor sets using 
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
	}

	void DirLightPassShader::UpdateDescriptorSets(const VulkanImageBufferObject * samplerPos, const VulkanImageBufferObject * samplerNormal, const VulkanImageBufferObject * samplerAlbedo)
	{
		// Update Descriptor set

		// descriptor info for samplerpos
		VkDescriptorImageInfo samplerposinfo{};
		samplerposinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		samplerposinfo.imageView = samplerPos->getImageView();
		samplerposinfo.sampler = samplerPos->getSampler();

		// descriptor info for samplernormal
		VkDescriptorImageInfo samplernorminfo{};
		samplernorminfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		samplernorminfo.imageView = samplerNormal->getImageView();
		samplernorminfo.sampler = samplerNormal->getSampler();

		// descriptor info for albedo
		VkDescriptorImageInfo albedoinfo{};
		albedoinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		albedoinfo.imageView = samplerAlbedo->getImageView();
		albedoinfo.sampler = samplerAlbedo->getSampler();

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_descriptorSet;
		descriptorWrites[0].dstBinding = 0; // we gave it at 0 binding
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pImageInfo = &samplerposinfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_descriptorSet;
		descriptorWrites[1].dstBinding = 1; // we gave it at 1 binding
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &samplernorminfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = m_descriptorSet;
		descriptorWrites[2].dstBinding = 2; // we gave it at 2 binding
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &albedoinfo;

		vkUpdateDescriptorSets(m_logicaldevice, (uint32_t) descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}

	void DirLightPassShader::Init(const VkRenderPass & renderpass)
	{
		// only when it has not created
		if (m_graphicPipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo vertinfo = CreateShaders_("./../Assets/Shaders/screenquad.vert.spv");
			vertinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertinfo.pName = "main";

			VkPipelineShaderStageCreateInfo fraginfo = CreateShaders_("./../Assets/Shaders/dirlightpass.frag.spv");
			fraginfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fraginfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertinfo, fraginfo };

			/* set up the fix pipeline info */
			FixedPipelineCreationTool fixedpipelineinfo{};
			SetUpFixedPipeline_(fixedpipelineinfo);

			/* set up the pipeline layout if have any */
			CreatePipelineLayout_();

			/* create the descriptor sets */
			CreateDescriptorSets_();
			
			/* create the graphics pipeline */
			VkGraphicsPipelineCreateInfo graphicspipeline_createinfo{};
			graphicspipeline_createinfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			graphicspipeline_createinfo.stageCount				= 2; // only vertex and fragment
			graphicspipeline_createinfo.pStages					= shaderStages;
			graphicspipeline_createinfo.pVertexInputState		= &fixedpipelineinfo.vertexInputInfo;
			graphicspipeline_createinfo.pInputAssemblyState		= &fixedpipelineinfo.inputAssembly;
			graphicspipeline_createinfo.pViewportState			= &fixedpipelineinfo.viewportState;
			graphicspipeline_createinfo.pRasterizationState		= &fixedpipelineinfo.rasterizer;
			graphicspipeline_createinfo.pMultisampleState		= &fixedpipelineinfo.multisampling;
			graphicspipeline_createinfo.pDepthStencilState		= &fixedpipelineinfo.depthStencil;
			graphicspipeline_createinfo.pColorBlendState		= &fixedpipelineinfo.colorBlending;
			graphicspipeline_createinfo.pDynamicState			= &fixedpipelineinfo.dynamicStateInfo; // ??
			graphicspipeline_createinfo.layout					= m_pipelineLayout;
			graphicspipeline_createinfo.renderPass				= renderpass;
			graphicspipeline_createinfo.subpass					= 0; // index of the subpass // take note of this
			graphicspipeline_createinfo.basePipelineHandle		= VK_NULL_HANDLE;
			graphicspipeline_createinfo.basePipelineIndex		= -1;

			DebugLog::EC(vkCreateGraphicsPipelines(m_logicaldevice, VK_NULL_HANDLE, 1, &graphicspipeline_createinfo, nullptr, &m_graphicPipeline));

			/* destroy the shader modules as they are useless now */
			if (vertinfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, vertinfo.module, nullptr); }
			if (fraginfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, fraginfo.module, nullptr); }
		}
	}

	void DirLightPassShader::SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline)
	{
		VkPipelineDepthStencilStateCreateInfo& depthStencil = fixedpipeline.depthStencil;
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp	= VK_COMPARE_OP_NEVER; // lower depth == closer
		depthStencil.depthBoundsTestEnable = VK_FALSE;
	}

	void DirLightPassShader::CreatePipelineLayout_()
	{
		// Descriptor layout
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		VkDescriptorSetLayoutBinding samplerPosLayoutBinding{};
		samplerPosLayoutBinding.binding = 0; // binding at 0 for worldfragpos texture
		samplerPosLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerPosLayoutBinding.descriptorCount = 1;
		samplerPosLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerPosLayoutBinding.pImmutableSamplers = nullptr; // related to image sampling

		VkDescriptorSetLayoutBinding samplerNormalLayoutBinding{};
		samplerNormalLayoutBinding.binding = 1; // binding at 1 for worldnormal texture
		samplerNormalLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerNormalLayoutBinding.descriptorCount = 1;
		samplerNormalLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerNormalLayoutBinding.pImmutableSamplers = nullptr; // related to image sampling

		VkDescriptorSetLayoutBinding samplerAlbedoLayoutBinding{};
		samplerAlbedoLayoutBinding.binding = 2; // binding at 2 for albedo texture
		samplerAlbedoLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerAlbedoLayoutBinding.descriptorCount = 1;
		samplerAlbedoLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerAlbedoLayoutBinding.pImmutableSamplers = nullptr; // related to image sampling

		std::array<VkDescriptorSetLayoutBinding, 3> bindings = { samplerPosLayoutBinding, samplerNormalLayoutBinding, samplerAlbedoLayoutBinding };

		VkDescriptorSetLayoutCreateInfo descriptorSetLayout_createinfo{};
		descriptorSetLayout_createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayout_createinfo.bindingCount = (uint32_t) bindings.size();
		descriptorSetLayout_createinfo.pBindings = bindings.data();

		DebugLog::EC(vkCreateDescriptorSetLayout(m_logicaldevice, &descriptorSetLayout_createinfo, nullptr, &m_descriptorSetLayout));

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		
		/* lastly pipeline layout creation */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		DebugLog::EC(vkCreatePipelineLayout(m_logicaldevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
	}

	void DirLightPassShader::CreateDescriptorSets_()
	{
		// Descriptor Sets

		// create the descriptor pool first
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = (uint32_t) poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 1;
		DebugLog::EC(vkCreateDescriptorPool(m_logicaldevice, &poolInfo, nullptr, &m_descriptorPool));

		// then allocate the descriptor set
		VkDescriptorSetAllocateInfo allocinfo{};
		allocinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocinfo.descriptorPool = m_descriptorPool;
		allocinfo.descriptorSetCount = 1;
		allocinfo.pSetLayouts = &m_descriptorSetLayout;
		vkAllocateDescriptorSets(m_logicaldevice, &allocinfo, &m_descriptorSet);
	}

	void DirLightPassShader::Destroy()
	{
		if (m_descriptorSetLayout != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorSetLayout(m_logicaldevice, m_descriptorSetLayout, nullptr);
			m_descriptorSetLayout = VK_NULL_HANDLE;
		}

		if (m_descriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(m_logicaldevice, m_descriptorPool, nullptr);
			m_descriptorPool = VK_NULL_HANDLE;
		}

		if (m_pipelineLayout != VK_NULL_HANDLE)
		{
			vkDestroyPipelineLayout(m_logicaldevice, m_pipelineLayout, nullptr);
			m_pipelineLayout = VK_NULL_HANDLE;
		}

		if (m_graphicPipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_logicaldevice, m_graphicPipeline, nullptr);
			m_graphicPipeline = VK_NULL_HANDLE;
		}
	}
}
