#include "TextShader.h"
#include "DebugLog.h"
#include "VulkanImageBufferObject.h"
#include "Vertex.h"
#include "SSBO.h"

namespace luna
{
	TextShader::TextShader()
	{
	}

	TextShader::~TextShader()
	{
		Destroy();
	}

	void TextShader::Init(const VkRenderPass & renderpass)
	{
		// only when it has not created
		if (m_Pipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo vertinfo = CreateShaders_(getAssetPath() + "Shaders/text_vert.spv");
			vertinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertinfo.pName = "main";

			VkPipelineShaderStageCreateInfo fraginfo = CreateShaders_(getAssetPath() + "Shaders/text_frag.spv");
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
			graphicspipeline_createinfo.subpass					= 0; // index of the subpass // take note of this
			graphicspipeline_createinfo.basePipelineHandle		= VK_NULL_HANDLE;
			graphicspipeline_createinfo.basePipelineIndex		= -1;

			DebugLog::EC(vkCreateGraphicsPipelines(m_logicaldevice, VK_NULL_HANDLE, 1, &graphicspipeline_createinfo, nullptr, &m_Pipeline));

			/* destroy the shader modules as they are useless now */
			if (vertinfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, vertinfo.module, nullptr); }
			if (fraginfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, fraginfo.module, nullptr); }
		}
	}

	void TextShader::Bind(const VkCommandBuffer & commandbuffer)
	{
		// Graphics Pipeline Binding (shaders binding)
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

		// bind the descriptor sets using 
		vkCmdBindDescriptorSets(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 1, &m_descriptorTool.descriptorSets[0], 0, nullptr);
	}

	void TextShader::SetDescriptors(const SSBO * ssbo, const VulkanImageBufferObject * font_image)
	{
		m_descriptorTool.Destroy(m_logicaldevice);

		// 2 kind of descriptors to send to
		// set up the layout for the shaders 
		const int totalbinding = 2;
		std::array<VulkanDescriptorLayoutInfo, totalbinding> layoutinfo{};

		// ssbo
		layoutinfo[0].binding = 0;
		layoutinfo[0].shaderstage = VK_SHADER_STAGE_VERTEX_BIT;
		layoutinfo[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutinfo[0].typeflags = 0;

		// image
		layoutinfo[1].binding = 1;
		layoutinfo[1].shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutinfo[1].typeflags = 1; // an image

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice, totalbinding, layoutinfo.data());

		// create the poolsize to hold all my descriptors
		const int totaldescriptors = 2; // total num of descriptors
		const int totalsets = 1; // total num of descriptor sets i will have

		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1;

		m_descriptorTool.SetUpDescriptorPools(m_logicaldevice, 2, poolSizes.data(), totalsets);
		m_descriptorTool.AddDescriptorSet(m_logicaldevice, 0);

		// first descriptor set update
		VkDescriptorBufferInfo ssboinfo{};
		ssboinfo.buffer = ssbo->getMainBuffer().buffer;
		ssboinfo.offset = 0;
		ssboinfo.range = ssbo->getSSBOTotalSize();
		VkDescriptorImageInfo fontimage{};
		fontimage.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL; // make sure layout to be shader read only optimal 
		fontimage.imageView = font_image->getImageView();
		fontimage.sampler = font_image->getSampler();

		std::array<VulkanDescriptorSetInfo, totalbinding> firstdescriptorset{};
		firstdescriptorset[0].layoutinfo = layoutinfo[0];
		firstdescriptorset[0].bufferinfo = ssboinfo;
		firstdescriptorset[1].layoutinfo = layoutinfo[1];
		firstdescriptorset[1].imageinfo = fontimage;

		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 0, totalbinding, firstdescriptorset.data());
	}

	void TextShader::UpdateDescriptor(const SSBO * ssbo)
	{
		// descriptor info for ssbo
		VkDescriptorBufferInfo ssboinfo{};
		ssboinfo.buffer = ssbo->getMainBuffer().buffer;
		ssboinfo.offset = 0;
		ssboinfo.range = ssbo->getSSBOTotalSize();

		// layout info
		VulkanDescriptorLayoutInfo layoutinfo{};
		layoutinfo.binding = 0;
		layoutinfo.shaderstage = VK_SHADER_STAGE_VERTEX_BIT;
		layoutinfo.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		layoutinfo.typeflags = 0;

		VulkanDescriptorSetInfo updateinfo{};
		updateinfo.bufferinfo = ssboinfo;
		updateinfo.layoutinfo = layoutinfo;

		// update the ssbo buffer
		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 0, updateinfo);
	}

	void TextShader::SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline)
	{
		// blending enable for fonts
		fixedpipeline.colorBlendAttachments.clear();
		fixedpipeline.colorBlendAttachments.resize(1);
		auto& blendAttachmentState = fixedpipeline.colorBlendAttachments[0];
		blendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		blendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;
		blendAttachmentState.blendEnable = VK_TRUE;
		blendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

		auto& colorblending = fixedpipeline.colorBlending;
		colorblending.attachmentCount = (uint32_t)fixedpipeline.colorBlendAttachments.size();
		colorblending.pAttachments = fixedpipeline.colorBlendAttachments.data();
		colorblending.logicOpEnable = VK_FALSE;
		colorblending.logicOp = VK_LOGIC_OP_COPY;

		// vertex attributes for fonts
		fixedpipeline.bindingDescription = FontVertex::getBindingDescription();
		fixedpipeline.attributeDescription.resize(FontVertex::getAttributeDescriptions().size());
		for (int i = 0; i < fixedpipeline.attributeDescription.size(); ++i)
		{
			fixedpipeline.attributeDescription[i] = FontVertex::getAttributeDescriptions()[i];
		}

		auto& vertexInputInfo = fixedpipeline.vertexInputInfo;
		vertexInputInfo.pVertexBindingDescriptions = &fixedpipeline.bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)fixedpipeline.attributeDescription.size();
		vertexInputInfo.pVertexAttributeDescriptions = fixedpipeline.attributeDescription.data();

		// no depth test
		VkPipelineDepthStencilStateCreateInfo& depthStencil = fixedpipeline.depthStencil;
		depthStencil.depthTestEnable = VK_FALSE;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.depthCompareOp	= VK_COMPARE_OP_NEVER; // lower depth == closer
		depthStencil.depthBoundsTestEnable = VK_FALSE;

		//fixedpipeline.rasterizer.polygonMode = VK_POLYGON_MODE_LINE;

	}

	void TextShader::CreatePipelineLayout_()
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
		pipelineLayoutInfo.pushConstantRangeCount = 0; // 0 push constant
		DebugLog::EC(vkCreatePipelineLayout(m_logicaldevice, &pipelineLayoutInfo, nullptr, &m_pipelineLayout));
	}

	void TextShader::Destroy()
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