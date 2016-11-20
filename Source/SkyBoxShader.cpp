#include "SkyBoxShader.h"
#include "DebugLog.h"
#include "UBO.h"
#include "VulkanTextureCube.h"
#include <array>

namespace luna
{
	SkyBoxShader::SkyBoxShader()
	{
	}

	SkyBoxShader::~SkyBoxShader()
	{
		Destroy();
	}

	void SkyBoxShader::Init(const VkRenderPass & renderpass, uint32_t subpassindex)
	{
		// only when it has not created
		if (m_Pipeline == VK_NULL_HANDLE)
		{
			/* create the shaders first */
			VkPipelineShaderStageCreateInfo vertinfo = CreateShaders_(getAssetPath() + "Shaders/skybox_vert.spv");
			vertinfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
			vertinfo.pName = "main";

			VkPipelineShaderStageCreateInfo fraginfo = CreateShaders_(getAssetPath() + "Shaders/skybox_frag.spv");
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
			graphicspipeline_createinfo.subpass					= subpassindex; // index of the subpass // take note of this
			graphicspipeline_createinfo.basePipelineHandle		= VK_NULL_HANDLE;
			graphicspipeline_createinfo.basePipelineIndex		= -1;

			DebugLog::EC(vkCreateGraphicsPipelines(m_logicaldevice, VK_NULL_HANDLE, 1, &graphicspipeline_createinfo, nullptr, &m_Pipeline));

			/* destroy the shader modules as they are useless now */
			if (vertinfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, vertinfo.module, nullptr); }
			if (fraginfo.module != VK_NULL_HANDLE) { vkDestroyShaderModule(m_logicaldevice, fraginfo.module, nullptr); }
		}
	}

	void SkyBoxShader::Bind(const VkCommandBuffer & commandbuffer)
	{
		// Graphics Pipeline Binding (shaders binding)
		vkCmdBindPipeline(commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_Pipeline);

		VkDescriptorSet descriptorSets[] = {
			m_descriptorTool.descriptorsInfo[0].descriptorSets[0], // first layout and its first descriptor sets
			m_descriptorTool.descriptorsInfo[1].descriptorSets[0] // second layout and its first descriptor sets
		};

		// bind the first layout and its first descriptor sets 
		vkCmdBindDescriptorSets(
			commandbuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipelineLayout, 0, 
			2, descriptorSets,
			0, nullptr
		);
	}

	void SkyBoxShader::SetDescriptors(const UBO * ubo, const VulkanImageBufferObject * cubemapimage)
	{
		m_descriptorTool.Destroy(m_logicaldevice);

		// set up the fist common layout for the shaders 
		// layout(set = 0, binding = x)
		VulkanDescriptorLayoutInfo layoutinfo0{};

		// ubo
		layoutinfo0.binding = 0;
		layoutinfo0.shaderstage = VK_SHADER_STAGE_VERTEX_BIT;
		layoutinfo0.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutinfo0.typeflags = 0;

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice, 1, &layoutinfo0);

		// material descriptor layout 
		// layout(set = 1, binding = x)
		VulkanDescriptorLayoutInfo layoutinfo1{};

		// cubemap texture
		layoutinfo1.binding = 0;
		layoutinfo1.shaderstage = VK_SHADER_STAGE_FRAGMENT_BIT;
		layoutinfo1.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		layoutinfo1.typeflags = 1; // an image

		m_descriptorTool.SetUpDescriptorLayout(m_logicaldevice, 1, &layoutinfo1);

		// create the poolsize to hold all my descriptors
		const int totaldescriptors = 2; // total num of descriptors
		const int totalsets = 2; // total num of descriptor sets i will have

		std::array<VkDescriptorPoolSize, 2> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = 1;
		poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		poolSizes[1].descriptorCount = 1;

		m_descriptorTool.SetUpDescriptorPools(m_logicaldevice, 2, poolSizes.data(), totalsets);
		m_descriptorTool.descriptorsInfo[0].descriptorSets.resize(1); // first layout has 1 descriptorsets
		m_descriptorTool.descriptorsInfo[1].descriptorSets.resize(1); // second layout has 1 descriptorsets

		m_descriptorTool.AddDescriptorSet(m_logicaldevice, 0, 0);
		m_descriptorTool.AddDescriptorSet(m_logicaldevice, 1, 0);

		// fist descriptorset layout de first descriptorset update
		VkDescriptorBufferInfo uboinfo{};
		uboinfo.buffer = ubo->getMainBuffer().buffer;
		uboinfo.offset = 0;
		uboinfo.range = ubo->getUboTotalSize();

		VulkanDescriptorSetInfo firstdescriptorset{};
		firstdescriptorset.layoutinfo = layoutinfo0;
		firstdescriptorset.bufferinfo = uboinfo;

		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 0, 0, 1, &firstdescriptorset);

		// second layout de descriptorset update
		VkDescriptorImageInfo cubemaptex{};
		cubemaptex.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		cubemaptex.imageView = cubemapimage->getImageView();
		cubemaptex.sampler = cubemapimage->getSampler();

		VulkanDescriptorSetInfo descriptorset{};
		descriptorset.layoutinfo = layoutinfo1;
		descriptorset.imageinfo = cubemaptex;

		m_descriptorTool.UpdateDescriptorSets(m_logicaldevice, 1, 0, 1, &descriptorset);
	}

	void SkyBoxShader::LoadModel(const VkCommandBuffer & cmdbuff, const glm::mat4 & model)
	{
		vkCmdPushConstants(cmdbuff, m_pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(model), &model);
	}

	void SkyBoxShader::SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline)
	{
		VkPipelineDepthStencilStateCreateInfo& depthStencil = fixedpipeline.depthStencil;
		depthStencil.depthWriteEnable = VK_FALSE;
		depthStencil.stencilTestEnable = VK_FALSE;
	}

	void SkyBoxShader::CreatePipelineLayout_()
	{
		/* push constant info */
		VkPushConstantRange pushconstant{};
		pushconstant.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushconstant.offset = 0;
		pushconstant.size = sizeof(glm::mat4);

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

	void SkyBoxShader::Destroy()
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
