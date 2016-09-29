#include "BasicShader.h"
#include "DebugLog.h"
#include "BasicUBO.h"
#include "BasicImage.h"

namespace luna
{
	BasicShader::BasicShader()
	{
	}

	BasicShader::~BasicShader()
	{
		Destroy();
	}

	void BasicShader::Bind(const VkCommandBuffer & commandbuffer)
	{
		// Graphics Pipeline Binding (shaders binding)
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline);

		// bind the descriptor sets using 
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorSet, 0, nullptr);
	}

	void BasicShader::LoadModelMatrix(const VkCommandBuffer& commandbuffer, const glm::mat4 & mat)
	{
		vkCmdPushConstants(commandbuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(mat), &mat);
	}

	void BasicShader::LoadColor(const VkCommandBuffer & commandbuffer, const glm::vec4 & color)
	{
		vkCmdPushConstants(commandbuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, sizeof(glm::mat4), sizeof(color), &color);
	}

	void BasicShader::Init(const VkRenderPass& renderpass)
	{
		// only when it has not created
		if (m_graphicPipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo vertinfo = CreateShaders_("./../Assets/Shaders/vertex.spv");
			vertinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertinfo.pName = "main";

			VkPipelineShaderStageCreateInfo fraginfo = CreateShaders_("./../Assets/Shaders/fragment.spv");
			fraginfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fraginfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertinfo, fraginfo };

			/* set up the fix pipeline info */
			FixedPipelineCreationInfo fixedpipelineinfo{};
			SetUpFixedPipeline(fixedpipelineinfo);

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
			//graphicspipeline_createinfo.pDynamicState			= &fixedpipelineinfo.dynamicStateInfo;
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

	void BasicShader::CreatePipelineLayout_()
	{
		// Descriptor layout
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr; // related to image sampling

		VkDescriptorSetLayoutBinding samplerLayoutBinding{};
		samplerLayoutBinding.binding = 1;
		samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerLayoutBinding.descriptorCount = 1;
		samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerLayoutBinding.pImmutableSamplers = nullptr; // related to image sampling

		std::array<VkDescriptorSetLayoutBinding, 2> bindings = { uboLayoutBinding, samplerLayoutBinding };

		VkDescriptorSetLayoutCreateInfo descriptorSetLayout_createinfo{};
		descriptorSetLayout_createinfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptorSetLayout_createinfo.bindingCount = (uint32_t) bindings.size();
		descriptorSetLayout_createinfo.pBindings = bindings.data();

		DebugLog::EC(vkCreateDescriptorSetLayout(m_logicaldevice, &descriptorSetLayout_createinfo, nullptr, &m_descriptorSetLayout));

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		VkPushConstantRange pushconstant{};
		pushconstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushconstant.offset = 0;
		pushconstant.size = sizeof(glm::mat4) + sizeof(glm::vec4);

		// Pipeline layout (uniform in shader)
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1; 
		pipelineLayoutInfo.pPushConstantRanges = &pushconstant;

		DebugLog::EC(vkCreatePipelineLayout(m_logicaldevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	}

	void BasicShader::CreateDescriptorSets_()
	{
		// Descriptor Sets
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		// create the descriptor pool first
		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1;

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


		// Update Descriptor set
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		// descriptor info for Uniform Buffer 
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = m_ubo->getMainBuffer();
		bufferInfo.offset = 0;
		bufferInfo.range = m_ubo->getUboTotalSize();

		// descriptor info for Image Sampler 
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = m_image->GetImageView();
		imageInfo.sampler = m_image->GetImageSampler();

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = m_descriptorSet;
		descriptorWrites[0].dstBinding = 0; // we gave it at 0 binding
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = m_descriptorSet;
		descriptorWrites[1].dstBinding = 1; // we gave it at 1 binding
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(m_logicaldevice, (uint32_t) descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	}

	void BasicShader::Destroy()
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