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
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

		// bind the descriptor sets using 
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorTool.descriptorSets[0], 0, nullptr);
	}

	void DirLightPassShader::SetDescriptors(const VulkanImageBufferObject * samplerPos, const VulkanImageBufferObject * samplerNormal, 
		const VulkanImageBufferObject * samplerAlbedo)
	{
		// 3 kind of descriptors to send to
		// set up the layout for the shaders 
		const int totalbinding = 3;
		std::array<VulkanDescriptorLayoutInfo, totalbinding> layoutinfo{};

		// sampler pos
		layoutinfo[0].binding = 0;
		layoutinfo[0].shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		layoutinfo[0].typeflags = 1; // an image

		// sampler norm
		layoutinfo[1].binding = 1;
		layoutinfo[1].shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo[1].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		layoutinfo[1].typeflags = 1; // an image

		// sampler albedo
		layoutinfo[2].binding = 2;
		layoutinfo[2].shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		layoutinfo[2].typeflags = 1; // an image

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice, totalbinding, layoutinfo.data());

		// create the poolsize to hold all my descriptors
		const int totaldescriptors = 3; // total num of descriptors
		const int totalsets = 1; // total num of descriptor sets i will have

		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		poolSize.descriptorCount = totaldescriptors;

		m_descriptorTool.SetUpDescriptorPools(m_logicaldevice, 1, &poolSize, totalsets);
		m_descriptorTool.AddDescriptorSet(m_logicaldevice, 0);

		// first descriptor set update
		VkDescriptorImageInfo samplerposinfo{};
		samplerposinfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		samplerposinfo.imageView = samplerPos->getImageView();
		samplerposinfo.sampler = samplerPos->getSampler();
		VkDescriptorImageInfo samplernorminfo{};
		samplernorminfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		samplernorminfo.imageView = samplerNormal->getImageView();
		samplernorminfo.sampler = samplerNormal->getSampler();
		VkDescriptorImageInfo albedoinfo{};
		albedoinfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		albedoinfo.imageView = samplerAlbedo->getImageView();
		albedoinfo.sampler = samplerAlbedo->getSampler();

		std::array<VulkanDescriptorSetInfo, totalbinding> firstdescriptorset{};
		firstdescriptorset[0].layoutinfo = layoutinfo[0];
		firstdescriptorset[0].imageinfo = samplerposinfo;
		firstdescriptorset[1].layoutinfo = layoutinfo[1];
		firstdescriptorset[1].imageinfo = samplernorminfo;
		firstdescriptorset[2].layoutinfo = layoutinfo[2];
		firstdescriptorset[2].imageinfo = albedoinfo;

		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 0, totalbinding, firstdescriptorset.data());
	}

	void DirLightPassShader::Init(const VkRenderPass & renderpass)
	{
		// only when it has not created
		if (m_Pipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo vertinfo = CreateShaders_(getAssetPath() + "Shaders/screenquad_vert.spv");
			vertinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertinfo.pName = "main";

			VkPipelineShaderStageCreateInfo fraginfo = CreateShaders_(getAssetPath() + "Shaders/dirlightsubpass_frag.spv");
			fraginfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
			fraginfo.pName = "main";

			VkPipelineShaderStageCreateInfo shaderStages[] = { vertinfo, fraginfo };

			/* set up the fix pipeline info */
			FixedPipelineCreationTool fixedpipelineinfo{};
			SetUpFixedPipeline_(fixedpipelineinfo);

			/* set up the pipeline layout if have any */
			CreatePipelineLayout_();
			
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
			graphicspipeline_createinfo.subpass					= 1; // index of the subpass // take note of this
			graphicspipeline_createinfo.basePipelineHandle		= VK_NULL_HANDLE;
			graphicspipeline_createinfo.basePipelineIndex		= -1;

			DebugLog::EC(vkCreateGraphicsPipelines(m_logicaldevice, VK_NULL_HANDLE, 1, &graphicspipeline_createinfo, nullptr, &m_Pipeline));

			/* destroy the shader modules as they are useless now */
			if (vertinfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, vertinfo.module, nullptr); }
			if (fraginfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, fraginfo.module, nullptr); }
		}
	}

	void DirLightPassShader::SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline)
	{
		// no depth test
		VkPipelineDepthStencilStateCreateInfo& depthStencil = fixedpipeline.depthStencil;
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp	= VK_COMPARE_OP_NEVER; // lower depth == closer
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_TRUE; // stencil test enable

		VkStencilOpState frontstate{};
		frontstate.compareOp = VK_COMPARE_OP_GREATER_OR_EQUAL; // the comparison operator used in the stencil test
		frontstate.failOp = VK_STENCIL_OP_KEEP; // the action performed on samples that fail the stencil test
		frontstate.depthFailOp = VK_STENCIL_OP_KEEP; // the action performed on samples that pass the stencil test and fail the depth test
		frontstate.passOp = VK_STENCIL_OP_REPLACE; // the action performed on samples that pass both the depth and stencil tests
		frontstate.writeMask = 0; // selects the bits of the unsigned integer stencil values updated by the stencil test in the stencil framebuffer attachment
		frontstate.compareMask = 0; // selects the bits of the unsigned integer stencil values participating in the stencil test
		frontstate.reference = 0; // is an integer reference value that is used in the unsigned stencil comparison

		depthStencil.front = frontstate;
		depthStencil.back = {}; // dun care about the back facing polygon

		// vertex attributes for screen quad
		fixedpipeline.bindingDescription = ScreenQuadVertex::getBindingDescription();
		fixedpipeline.attributeDescription.resize(ScreenQuadVertex::getAttributeDescriptions().size());
		for (int i = 0; i < fixedpipeline.attributeDescription.size(); ++i)
		{
			fixedpipeline.attributeDescription[i] = ScreenQuadVertex::getAttributeDescriptions()[i];
		}

		auto& vertexInputInfo = fixedpipeline.vertexInputInfo;
		vertexInputInfo.pVertexBindingDescriptions = &fixedpipeline.bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)fixedpipeline.attributeDescription.size();
		vertexInputInfo.pVertexAttributeDescriptions = fixedpipeline.attributeDescription.data();

		// dynamic state 
		fixedpipeline.dynamicState.resize(5);
		fixedpipeline.dynamicState[0] = VK_DYNAMIC_STATE_VIEWPORT;
		fixedpipeline.dynamicState[1] = VK_DYNAMIC_STATE_SCISSOR;
		fixedpipeline.dynamicState[2] = VK_DYNAMIC_STATE_STENCIL_REFERENCE;
		fixedpipeline.dynamicState[3] = VK_DYNAMIC_STATE_STENCIL_WRITE_MASK;
		fixedpipeline.dynamicState[4] = VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK;

		fixedpipeline.dynamicStateInfo.dynamicStateCount = (uint32_t)fixedpipeline.dynamicState.size();
		fixedpipeline.dynamicStateInfo.pDynamicStates = fixedpipeline.dynamicState.data();
	}

	void DirLightPassShader::CreatePipelineLayout_()
	{
		if (m_descriptorTool.descriptorSetLayout == VK_NULL_HANDLE)
		{
			DebugLog::throwEx("descriptor set layout not init");
		}

		/* lastly pipeline layout creation */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorTool.descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		DebugLog::EC(vkCreatePipelineLayout(m_logicaldevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
	}

	void DirLightPassShader::Destroy()
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
