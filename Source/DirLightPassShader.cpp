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
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorTool.descriptorSets, 0, nullptr);
	}

	void DirLightPassShader::SetDescriptors(const VulkanImageBufferObject * samplerPos, const VulkanImageBufferObject * samplerNormal, const VulkanImageBufferObject * samplerAlbedo)
	{
		// descriptor info for samplerpos
		VkDescriptorImageInfo samplerposinfo{};
		samplerposinfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		samplerposinfo.imageView = samplerPos->getImageView();
		samplerposinfo.sampler = samplerPos->getSampler();

		// descriptor info for samplernormal
		VkDescriptorImageInfo samplernorminfo{};
		samplernorminfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		samplernorminfo.imageView = samplerNormal->getImageView();
		samplernorminfo.sampler = samplerNormal->getSampler();

		// descriptor info for albedo
		VkDescriptorImageInfo albedoinfo{};
		albedoinfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		albedoinfo.imageView = samplerAlbedo->getImageView();
		albedoinfo.sampler = samplerAlbedo->getSampler();

		m_descriptorTool.descriptors.resize(3);

		auto& samplerposdescriptor = m_descriptorTool.descriptors[0];
		samplerposdescriptor.binding = 0;
		samplerposdescriptor.imageinfo = samplerposinfo;
		samplerposdescriptor.shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplerposdescriptor.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		samplerposdescriptor.typeflags = 1; // an image

		auto& samplernormdescriptor = m_descriptorTool.descriptors[1];
		samplernormdescriptor.binding = 1;
		samplernormdescriptor.imageinfo = samplernorminfo;
		samplernormdescriptor.shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		samplernormdescriptor.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		samplernormdescriptor.typeflags = 1; // an image

		auto& albedodescriptor = m_descriptorTool.descriptors[2];
		albedodescriptor.binding = 2;
		albedodescriptor.imageinfo = albedoinfo;
		albedodescriptor.shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		albedodescriptor.type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		albedodescriptor.typeflags = 1; // an image

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice);
		m_descriptorTool.SetUpDescriptorSets(m_logicaldevice);
		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice);
	}

	void DirLightPassShader::Init(const VkRenderPass & renderpass)
	{
		// only when it has not created
		if (m_graphicPipeline == VK_NULL_HANDLE)
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

			DebugLog::EC(vkCreateGraphicsPipelines(m_logicaldevice, VK_NULL_HANDLE, 1, &graphicspipeline_createinfo, nullptr, &m_graphicPipeline));

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
		frontstate.compareOp = VK_COMPARE_OP_EQUAL; // the comparison operator used in the stencil test
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

		if (m_graphicPipeline != VK_NULL_HANDLE)
		{
			vkDestroyPipeline(m_logicaldevice, m_graphicPipeline, nullptr);
			m_graphicPipeline = VK_NULL_HANDLE;
		}
	}
}
