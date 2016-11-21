#include "GBufferSubpassShader.h"
#include "SSBO.h"
#include "UBO.h"
#include "DebugLog.h"
#include "VulkanImageBufferObject.h"
#include <array>

namespace luna
{
	GBufferSubpassShader::GBufferSubpassShader()
	{
	}

	GBufferSubpassShader::~GBufferSubpassShader()
	{
		Destroy();
	}

	void GBufferSubpassShader::Bind(const VkCommandBuffer & commandbuffer)
	{
		// Graphics Pipeline Binding (shaders binding)
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);
	}

	void GBufferSubpassShader::BindTexture(const VkCommandBuffer & commandbuffer, const int & whichset)
	{
		VkDescriptorSet descriptorSets[] = {
			m_descriptorTool.descriptorsInfo[0].descriptorSets[0], // first layout and its first descriptor sets
			m_descriptorTool.descriptorsInfo[1].descriptorSets[whichset] // second layout and one of its descriptor sets
		};

		// bind the first layout and its first descriptor sets 
		vkCmdBindDescriptorSets(
			commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 
			2, descriptorSets,
			0, nullptr
		);
	}

	void GBufferSubpassShader::SetDescriptors(const UBO* ubo, const SSBO* ssbo, const std::vector<VulkanImageBufferObject*>& diffuseTexs)
	{
		m_descriptorTool.Destroy(m_logicaldevice);

		// set up the fist common layout for the shaders 
		// layout(set = 0, binding = x)
		std::array<VulkanDescriptorLayoutInfo, 2> layoutinfo0{};

		// ubo
		layoutinfo0[0].binding = 0;
		layoutinfo0[0].shaderstage = VK_SHADER_STAGE_VERTEX_BIT;
		layoutinfo0[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutinfo0[0].typeflags = 0;

		// ssbo
		layoutinfo0[1].binding = 1;
		layoutinfo0[1].shaderstage = VK_SHADER_STAGE_VERTEX_BIT;
		layoutinfo0[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutinfo0[1].typeflags = 0;

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice, 2, layoutinfo0.data());

		// material descriptor layout 
		// layout(set = 1, binding = x)
		VulkanDescriptorLayoutInfo layoutinfo1{};

		// diffuse texture
		layoutinfo1.binding = 0;
		layoutinfo1.shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo1.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutinfo1.typeflags = 1; // an image

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice, 1, &layoutinfo1);

		// create the poolsize to hold all my descriptors
		const int totaldescriptors = 3; // total num of descriptors
		const int totalNumDiffTex = static_cast<int>(diffuseTexs.size());
		const int totalsets = 1 + totalNumDiffTex; // total num of descriptor sets i will have

		// 3 different kind of descriptor sets type
		std::array<VkDescriptorPoolSize, 3> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[1].descriptorCount = 1;
		poolSizes[2].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[2].descriptorCount = static_cast<uint32_t>(totalNumDiffTex);

		m_descriptorTool.SetUpDescriptorPools(m_logicaldevice, 3, poolSizes.data(), totalsets);
		m_descriptorTool.descriptorsInfo[0].descriptorSets.resize(1); // first layout has 1 descriptorsets
		m_descriptorTool.descriptorsInfo[1].descriptorSets.resize(totalNumDiffTex); // second layout has num descriptorsets based on how many diff texs i have
		
		// first layout descriptor set adding
		m_descriptorTool.AddDescriptorSet(m_logicaldevice, 0, 0);
		// second layout descriptor sets adding
		for (int i = 0; i < totalNumDiffTex; ++i)
		{
			m_descriptorTool.AddDescriptorSet(m_logicaldevice, 1, i);
		}

		// fist descriptorset layout de first descriptorset update
		VkDescriptorBufferInfo uboinfo{};
		uboinfo.buffer = ubo->getMainBuffer().buffer;
		uboinfo.offset = 0;
		uboinfo.range = ubo->getUboTotalSize();
		VkDescriptorBufferInfo ssboinfo{};
		ssboinfo.buffer = ssbo->getMainBuffer().buffer;
		ssboinfo.offset = 0;
		ssboinfo.range = ssbo->getSSBOTotalSize();
		
		std::array<VulkanDescriptorSetInfo, 2> firstdescriptorset{};
		firstdescriptorset[0].layoutinfo = layoutinfo0[0];
		firstdescriptorset[0].bufferinfo = uboinfo;
		firstdescriptorset[1].layoutinfo = layoutinfo0[1];
		firstdescriptorset[1].bufferinfo = ssboinfo;
		
		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 0, 0, 2, firstdescriptorset.data());

		// second layout de descriptorset update
		for (int i = 0; i < totalNumDiffTex; ++i)
		{
			VkDescriptorImageInfo albedoinfo{};
			albedoinfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			albedoinfo.imageView = diffuseTexs[i]->getImageView();
			albedoinfo.sampler = diffuseTexs[i]->getSampler();

			VulkanDescriptorSetInfo descriptorset{};
			descriptorset.layoutinfo = layoutinfo1;
			descriptorset.imageinfo = albedoinfo;

			m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 1, i, 1, &descriptorset);
		}
	}

	void GBufferSubpassShader::UpdateDescriptor(const SSBO* ssbo)
	{
		// descriptor info for ssbo
		VkDescriptorBufferInfo ssboinfo{};
		ssboinfo.buffer = ssbo->getMainBuffer().buffer;
		ssboinfo.offset = 0;
		ssboinfo.range = ssbo->getSSBOTotalSize();

		// layout info
		VulkanDescriptorLayoutInfo layoutinfo{};
		layoutinfo.binding = 1;
		layoutinfo.shaderstage = VK_SHADER_STAGE_VERTEX_BIT;
		layoutinfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutinfo.typeflags = 0;

		VulkanDescriptorSetInfo updateinfo{};
		updateinfo.bufferinfo = ssboinfo;
		updateinfo.layoutinfo = layoutinfo;

		// update the ssbo buffer
		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 0, 0, updateinfo);
	}

	void GBufferSubpassShader::LoadObjectOffset(const VkCommandBuffer& commandbuffer, const int & offset)
	{
		vkCmdPushConstants(commandbuffer, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(offset), &offset);
	}

	void GBufferSubpassShader::Init(const VkRenderPass & renderpass, uint32_t subpassindex)
	{
		// only when it has not created
		if (m_Pipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo vertinfo = CreateShaders_(getAssetPath() + "Shaders/gbuffersubpass_vert.spv");
			vertinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertinfo.pName = "main";

			VkPipelineShaderStageCreateInfo fraginfo = CreateShaders_(getAssetPath() + "Shaders/gbuffersubpass_frag.spv");
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
			graphicspipeline_createinfo.subpass					= subpassindex; // index of the subpass // take note of this
			graphicspipeline_createinfo.basePipelineHandle		= VK_NULL_HANDLE;
			graphicspipeline_createinfo.basePipelineIndex		= -1;

			DebugLog::EC(vkCreateGraphicsPipelines(m_logicaldevice, VK_NULL_HANDLE, 1, &graphicspipeline_createinfo, nullptr, &m_Pipeline));

			/* destroy the shader modules as they are useless now */
			if (vertinfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, vertinfo.module, nullptr); }
			if (fraginfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, fraginfo.module, nullptr); }
		}
	}

	void GBufferSubpassShader::SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline)
	{
		// Blend attachment states required for all color attachments
		// This is important, as color write mask will otherwise be 0x0 and you
		// won't see anything rendered to the attachment
		fixedpipeline.colorBlendAttachments.clear();
		fixedpipeline.colorBlendAttachments.resize(2);

		fixedpipeline.colorBlendAttachments[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		fixedpipeline.colorBlendAttachments[0].blendEnable = VK_FALSE;
		fixedpipeline.colorBlendAttachments[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		fixedpipeline.colorBlendAttachments[1].blendEnable = VK_FALSE;

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

	void GBufferSubpassShader::CreatePipelineLayout_()
	{
		/* push constant info */
		VkPushConstantRange pushconstant{};
		pushconstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushconstant.offset = 0;
		pushconstant.size = sizeof(int);
	
		std::vector<VkDescriptorSetLayout> setlayouts;
		setlayouts.resize(m_descriptorTool.descriptorsInfo.size());
		for (int i = 0; i < m_descriptorTool.descriptorsInfo.size(); ++i)
		{
			setlayouts[i] = m_descriptorTool.descriptorsInfo[i].descriptorSetLayout;
		}

		/* lastly pipeline layout creation */
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(setlayouts.size()); // how many descriptor layout i have
		pipelineLayoutInfo.pSetLayouts = setlayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1; 
		pipelineLayoutInfo.pPushConstantRanges = &pushconstant;

		DebugLog::EC(vkCreatePipelineLayout(m_logicaldevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
	}

	void GBufferSubpassShader::Destroy()
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
