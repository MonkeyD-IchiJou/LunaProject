#include "FinalPassShader.h"
#include "DebugLog.h"
#include "VulkanImageBufferObject.h"
#include "Vertex.h"

namespace luna
{

	FinalPassShader::FinalPassShader()
	{
	}

	FinalPassShader::~FinalPassShader()
	{
		Destroy();
	}

	void FinalPassShader::Bind(const VkCommandBuffer & commandbuffer)
	{
		// Graphics Pipeline Binding (shaders binding)
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline);

		// bind the descriptor sets using 
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
	}

	void FinalPassShader::UpdateDescriptorSets(const VulkanImageBufferObject * finalimage)
	{
		// Update Descriptor set

		// descriptor info for samplerpos
		VkDescriptorImageInfo samplerposinfo{};
		samplerposinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // make sure layout to be shader read only optimal 
		samplerposinfo.imageView = finalimage->getImageView();
		samplerposinfo.sampler = finalimage->getSampler();

		VkWriteDescriptorSet descriptorWrites{};
		descriptorWrites.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites.dstSet = m_descriptorSet;
		descriptorWrites.dstBinding = 0; // we gave it at 0 binding
		descriptorWrites.dstArrayElement = 0;
		descriptorWrites.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites.descriptorCount = 1;
		descriptorWrites.pImageInfo = &samplerposinfo;

		vkUpdateDescriptorSets(m_logicaldevice, 1, &descriptorWrites, 0, nullptr);
	}

	void FinalPassShader::Init(const VkRenderPass & renderpass)
	{
		// only when it has not created
		if (m_graphicPipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo vertinfo = CreateShaders_("./../Assets/Shaders/screenquad_vert.spv");
			vertinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertinfo.pName = "main";

			VkPipelineShaderStageCreateInfo fraginfo = CreateShaders_("./../Assets/Shaders/finalpass_frag.spv");
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

	void FinalPassShader::SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline)
	{
		// no depth test
		VkPipelineDepthStencilStateCreateInfo& depthStencil = fixedpipeline.depthStencil;
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp	= VK_COMPARE_OP_NEVER; // lower depth == closer
		depthStencil.depthBoundsTestEnable = VK_FALSE;
	}

	void FinalPassShader::CreatePipelineLayout_()
	{
		// Descriptor layout
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		VkDescriptorSetLayoutBinding samplerPosLayoutBinding{};
		samplerPosLayoutBinding.binding = 0; // binding at 0 for finalcolor texture
		samplerPosLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerPosLayoutBinding.descriptorCount = 1;
		samplerPosLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerPosLayoutBinding.pImmutableSamplers = nullptr; // related to image sampling

		VkDescriptorSetLayoutCreateInfo descriptorSetLayout_createinfo{};
		descriptorSetLayout_createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayout_createinfo.bindingCount = 1;
		descriptorSetLayout_createinfo.pBindings = &samplerPosLayoutBinding;

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

	void FinalPassShader::CreateDescriptorSets_()
	{
		// Descriptor Sets

		// create the descriptor pool first
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
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

	void FinalPassShader::Destroy()
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
