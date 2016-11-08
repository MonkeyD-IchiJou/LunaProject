#include "DirLightPassShader.h"
#include "DebugLog.h"
#include "VulkanImageBufferObject.h"
#include "Vertex.h"
#include "UBO.h"

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
		vkCmdBindDescriptorSets(
			commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 
			1, &m_descriptorTool.descriptorsInfo[0].descriptorSets[0], 
			0, nullptr
		);
	}

	void DirLightPassShader::LoadDirLightPos(const VkCommandBuffer& commandbuffer, const glm::vec3 & pos)
	{
		vkCmdPushConstants(commandbuffer, m_pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(pos), &pos);
	}

	void DirLightPassShader::SetDescriptors(
		const VulkanImageBufferObject* color0, 
		const VulkanImageBufferObject* color1, 
		const VulkanImageBufferObject* color2,
		const VulkanImageBufferObject* color3,
		const UBO* ubo_pointlights)
	{
		// 5 kind of descriptors to send to
		// set up the layout for the shaders 
		const int totalbinding = 5;
		std::array<VulkanDescriptorLayoutInfo, totalbinding> layoutinfo{};

		// color0
		layoutinfo[0].binding = 0;
		layoutinfo[0].shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		layoutinfo[0].typeflags = 1; // an image

		// color1
		layoutinfo[1].binding = 1;
		layoutinfo[1].shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo[1].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		layoutinfo[1].typeflags = 1; // an image

		// color2
		layoutinfo[2].binding = 2;
		layoutinfo[2].shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo[2].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		layoutinfo[2].typeflags = 1; // an image

		// color3
		layoutinfo[3].binding = 3;
		layoutinfo[3].shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo[3].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		layoutinfo[3].typeflags = 1; // an image

		layoutinfo[4].binding = 4;
		layoutinfo[4].shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo[4].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutinfo[4].typeflags = 0; // a buffer

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice, totalbinding, layoutinfo.data());

		// create the poolsize to hold all my descriptors
		const int totaldescriptors = 5; // total num of descriptors
		const int totalsets = 1; // total num of descriptor sets i will have

		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
		poolSizes[0].descriptorCount = 4; // 4 input attachments
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[1].descriptorCount = 1;

		m_descriptorTool.SetUpDescriptorPools(m_logicaldevice, 2, poolSizes.data(), totalsets);
		m_descriptorTool.descriptorsInfo[0].descriptorSets.resize(1); // first layout has 1 descriptorsets
		m_descriptorTool.AddDescriptorSet(m_logicaldevice, 0, 0);

		// first descriptor set update
		VkDescriptorImageInfo color0info{};
		color0info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		color0info.imageView = color0->getImageView();
		color0info.sampler = color0->getSampler();
		VkDescriptorImageInfo color1info{};
		color1info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		color1info.imageView = color1->getImageView();
		color1info.sampler = color1->getSampler();
		VkDescriptorImageInfo color2info{};
		color2info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		color2info.imageView = color2->getImageView();
		color2info.sampler = color2->getSampler();
		VkDescriptorImageInfo color3info{};
		color3info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
		color3info.imageView = color3->getImageView();
		color3info.sampler = color3->getSampler();
		VkDescriptorBufferInfo uboinfo{};
		uboinfo.buffer = ubo_pointlights->getMainBuffer().buffer;
		uboinfo.offset = 0;
		uboinfo.range = ubo_pointlights->getUboTotalSize();

		std::array<VulkanDescriptorSetInfo, totalbinding> firstdescriptorset{};
		firstdescriptorset[0].layoutinfo = layoutinfo[0];
		firstdescriptorset[0].imageinfo = color0info;
		firstdescriptorset[1].layoutinfo = layoutinfo[1];
		firstdescriptorset[1].imageinfo = color1info;
		firstdescriptorset[2].layoutinfo = layoutinfo[2];
		firstdescriptorset[2].imageinfo = color2info;
		firstdescriptorset[3].layoutinfo = layoutinfo[3];
		firstdescriptorset[3].imageinfo = color3info;
		firstdescriptorset[4].layoutinfo = layoutinfo[4];
		firstdescriptorset[4].bufferinfo = uboinfo;

		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 0, 0, totalbinding, firstdescriptorset.data());
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
		/* push constant info */
		VkPushConstantRange pushconstant{};
		pushconstant.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		pushconstant.offset = 0;
		pushconstant.size = sizeof(glm::vec3);

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
