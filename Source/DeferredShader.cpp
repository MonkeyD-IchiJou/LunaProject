#include "DeferredShader.h"
#include "SSBO.h"
#include "UBO.h"
#include "DebugLog.h"
#include "VulkanImageBufferObject.h"
#include <array>

namespace luna
{
	DeferredShader::DeferredShader()
	{
	}

	DeferredShader::~DeferredShader()
	{
		Destroy();
	}

	void DeferredShader::Bind(const VkCommandBuffer & commandbuffer)
	{
		// Graphics Pipeline Binding (shaders binding)
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline);

		// bind the descriptor sets using 
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorTool.descriptorSets, 0, nullptr);
	}

	void DeferredShader::SetDescriptors(const UBO* ubo, const SSBO* ssbo, const VulkanImageBufferObject* image)
	{
		m_descriptorTool.Destroy(m_logicaldevice);

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

		m_descriptorTool.descriptors.resize(3);

		auto& ubodescriptor = m_descriptorTool.descriptors[0];
		ubodescriptor.binding = 0;
		ubodescriptor.bufferinfo = uboinfo;
		ubodescriptor.shaderstage = VK_SHADER_STAGE_VERTEX_BIT;
		ubodescriptor.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		ubodescriptor.typeflags = 0;

		auto& ssbodescriptor = m_descriptorTool.descriptors[1];
		ssbodescriptor.binding = 1;
		ssbodescriptor.bufferinfo = ssboinfo;
		ssbodescriptor.shaderstage = VK_SHADER_STAGE_VERTEX_BIT;
		ssbodescriptor.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		ssbodescriptor.typeflags = 0;

		auto& albedoimagedescriptor = m_descriptorTool.descriptors[2];
		albedoimagedescriptor.binding = 2;
		albedoimagedescriptor.imageinfo = albedoinfo;
		albedoimagedescriptor.shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		albedoimagedescriptor.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		albedoimagedescriptor.typeflags = 1; // an image

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice);
		m_descriptorTool.SetUpDescriptorSets(m_logicaldevice);
		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice);
	}

	void DeferredShader::UpdateDescriptor(const SSBO* ssbo)
	{
		// descriptor info for ssbo
		VkDescriptorBufferInfo ssboinfo{};
		ssboinfo.buffer = ssbo->getMainBuffer().buffer;
		ssboinfo.offset = 0;
		ssboinfo.range = ssbo->getSSBOTotalSize();

		auto& ssbodescriptor = m_descriptorTool.descriptors[1];
		ssbodescriptor.bufferinfo = ssboinfo;

		// update the ssbo buffer
		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 1);
	}

	void DeferredShader::LoadObjectOffset(const VkCommandBuffer& commandbuffer, const int & offset)
	{
		vkCmdPushConstants(commandbuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(offset), &offset);
	}

	void DeferredShader::Init(const VkRenderPass & renderpass)
	{
		// only when it has not created
		if (m_graphicPipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo vertinfo = CreateShaders_(getAssetPath() + "Shaders/deferred_vert.spv");
			vertinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertinfo.pName = "main";

			VkPipelineShaderStageCreateInfo fraginfo = CreateShaders_(getAssetPath() + "Shaders/deferred_frag.spv");
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

	void DeferredShader::SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline)
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

		// stencil enable
		VkPipelineDepthStencilStateCreateInfo& depthStencil = fixedpipeline.depthStencil;
		depthStencil.stencilTestEnable = VK_TRUE;

		VkStencilOpState frontstate{};
		frontstate.compareOp = VK_COMPARE_OP_ALWAYS; // the comparison operator used in the stencil test
		frontstate.failOp = VK_STENCIL_OP_KEEP; // the action performed on samples that fail the stencil test
		frontstate.depthFailOp = VK_STENCIL_OP_KEEP; // the action performed on samples that pass the stencil test and fail the depth test
		frontstate.passOp = VK_STENCIL_OP_REPLACE; // the action performed on samples that pass both the depth and stencil tests
		frontstate.writeMask = 0; // selects the bits of the unsigned integer stencil values updated by the stencil test in the stencil framebuffer attachment
		frontstate.compareMask = 0; // selects the bits of the unsigned integer stencil values participating in the stencil test
		frontstate.reference = 0; // is an integer reference value that is used in the unsigned stencil comparison

		depthStencil.front = frontstate;
		depthStencil.back = {}; // dun care about the back facing polygon

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

	void DeferredShader::CreatePipelineLayout_()
	{
		if (m_descriptorTool.descriptorSetLayout == VK_NULL_HANDLE)
		{
			DebugLog::throwEx("descriptor set layout not init");
		}

		/* push constant info */
		VkPushConstantRange pushconstant{};
		pushconstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushconstant.offset = 0;
		pushconstant.size = sizeof(int);
	
		/* lastly pipeline layout creation */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 1;
		pipelineLayoutInfo.pSetLayouts = &m_descriptorTool.descriptorSetLayout;
		pipelineLayoutInfo.pushConstantRangeCount = 1; 
		pipelineLayoutInfo.pPushConstantRanges = &pushconstant;

		DebugLog::EC(vkCreatePipelineLayout(m_logicaldevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
	}

	void DeferredShader::Destroy()
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
