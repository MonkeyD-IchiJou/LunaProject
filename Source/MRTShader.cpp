#include "MRTShader.h"
#include "SSBO.h"
#include "UBO.h"
#include "DebugLog.h"
#include "VulkanImageBufferObject.h"
#include <array>

namespace luna
{
	MRTShader::MRTShader()
	{
	}

	MRTShader::~MRTShader()
	{
		Destroy();
	}

	void MRTShader::Bind(const VkCommandBuffer & commandbuffer)
	{
		// Graphics Pipeline Binding (shaders binding)
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline);

		// bind the descriptor sets using 
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
	}

	void MRTShader::UpdateDescriptorSets(const UBO* ubo, const SSBO* ssbo, const VulkanImageBufferObject* image)
	{
		// Update Descriptor set

		// descriptor info for Uniform Buffer
		VkDescriptorBufferInfo uboinfo{};
		uboinfo.buffer = ubo->getMainBuffer().buffer;
		uboinfo.offset = 0;
		uboinfo.range = ubo->getUboTotalSize();

		// descriptor info for ssbo
		VkDescriptorBufferInfo ssboinfo{};
		ssboinfo.buffer = ssbo->getMainBuffer().buffer;
		ssboinfo.offset = 0;
		ssboinfo.range = ssbo->getSSBOTotalSize();

		// descriptor info for albedo texture
		VkDescriptorImageInfo albedoinfo{};
		albedoinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		albedoinfo.imageView = image->getImageView();
		albedoinfo.sampler = image->getSampler();

		std::array<VkWriteDescriptorSet, 3> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_descriptorSet;
		descriptorWrites[0].dstBinding = 0; // we gave it at 0 binding
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &uboinfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_descriptorSet;
		descriptorWrites[1].dstBinding = 1; // we gave it at 1 binding
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &ssboinfo;

		descriptorWrites[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[2].dstSet = m_descriptorSet;
		descriptorWrites[2].dstBinding = 2; // we gave it at 2 binding
		descriptorWrites[2].dstArrayElement = 0;
		descriptorWrites[2].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[2].descriptorCount = 1;
		descriptorWrites[2].pImageInfo = &albedoinfo;

		vkUpdateDescriptorSets(m_logicaldevice, (uint32_t) descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
	}

	void MRTShader::RewriteSSBODescriptorSets(const SSBO* ssbo)
	{
		// descriptor info for ssbo
		VkDescriptorBufferInfo ssboinfo{};
		ssboinfo.buffer = ssbo->getMainBuffer().buffer;
		ssboinfo.offset = 0;
		ssboinfo.range = ssbo->getSSBOTotalSize();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_descriptorSet;
		descriptorWrite.dstBinding = 1; // we gave it at 1 binding
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &ssboinfo;

		vkUpdateDescriptorSets(m_logicaldevice, 1, &descriptorWrite, 0, nullptr);
	}

	void MRTShader::LoadObjectOffset(const VkCommandBuffer& commandbuffer, const int & offset)
	{
		vkCmdPushConstants(commandbuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(offset), &offset);
	}

	void MRTShader::Init(const VkRenderPass & renderpass)
	{
		// only when it has not created
		if (m_graphicPipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo vertinfo = CreateShaders_("./../Assets/Shaders/mrt_vert.spv");
			vertinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertinfo.pName = "main";

			VkPipelineShaderStageCreateInfo fraginfo = CreateShaders_("./../Assets/Shaders/mrt_frag.spv");
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
			graphicspipeline_createinfo.pDynamicState			= &fixedpipelineinfo.dynamicStateInfo;
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

	void MRTShader::SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline)
	{
		// Blend attachment states required for all color attachments
		// This is important, as color write mask will otherwise be 0x0 and you
		// won't see anything rendered to the attachment
		fixedpipeline.colorBlendAttachments.clear();
		fixedpipeline.colorBlendAttachments.resize(3);

		fixedpipeline.colorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		fixedpipeline.colorBlendAttachments[0].blendEnable = VK_FALSE;
		fixedpipeline.colorBlendAttachments[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		fixedpipeline.colorBlendAttachments[1].blendEnable = VK_FALSE;
		fixedpipeline.colorBlendAttachments[2].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		fixedpipeline.colorBlendAttachments[2].blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo& colorBlending = fixedpipeline.colorBlending;
		colorBlending.logicOp = VK_LOGIC_OP_CLEAR;
		colorBlending.attachmentCount = static_cast<uint32_t>(fixedpipeline.colorBlendAttachments.size());
		colorBlending.pAttachments = fixedpipeline.colorBlendAttachments.data();
	}

	void MRTShader::CreatePipelineLayout_()
	{
		// Descriptor layout
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0; // binding at 0 for uniform buffer
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // related to image sampling

		VkDescriptorSetLayoutBinding ssboLayoutBinding{};
		ssboLayoutBinding.binding = 1; // binding at 1 for storage buffer
		ssboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		ssboLayoutBinding.descriptorCount = 1;
		ssboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		ssboLayoutBinding.pImmutableSamplers = nullptr; // related to image sampling

		VkDescriptorSetLayoutBinding albedotexLayoutBinding{};
		albedotexLayoutBinding.binding = 2; // binding at 2 for albedo texture
		albedotexLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		albedotexLayoutBinding.descriptorCount = 1;
		albedotexLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		albedotexLayoutBinding.pImmutableSamplers = nullptr; // related to image sampling

		std::array<VkDescriptorSetLayoutBinding, 3> bindings = { uboLayoutBinding, ssboLayoutBinding, albedotexLayoutBinding };

		VkDescriptorSetLayoutCreateInfo descriptorSetLayout_createinfo{};
		descriptorSetLayout_createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayout_createinfo.bindingCount = (uint32_t) bindings.size();
		descriptorSetLayout_createinfo.pBindings = bindings.data();

		DebugLog::EC(vkCreateDescriptorSetLayout(m_logicaldevice, &descriptorSetLayout_createinfo, nullptr, &m_descriptorSetLayout));

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	
		/* push constant info */
		VkPushConstantRange pushconstant{};
		pushconstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushconstant.offset = 0;
		pushconstant.size = sizeof(int);
	
		/* lastly pipeline layout creation */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1; 
		pipelineLayoutInfo.pPushConstantRanges = &pushconstant;

		DebugLog::EC(vkCreatePipelineLayout(m_logicaldevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
	}

	void MRTShader::CreateDescriptorSets_()
	{
		// Descriptor Sets
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		// create the descriptor pool first
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
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

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	}

	void MRTShader::Destroy()
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
