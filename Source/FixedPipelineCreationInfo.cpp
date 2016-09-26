#include "FixedPipelineCreationInfo.h"
#include "Vertex.h"
#include "WinNative.h"

namespace luna
{

	FixedPipelineCreationInfo::FixedPipelineCreationInfo()
	{
		// Vertex Input
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		bindingDescription = Vertex::getBindingDescription();
		attributeDescription = Vertex::getAttributeDescriptions();

		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)attributeDescription.size();
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescription.data();

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		// Input Assembly
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		inputAssembly.sType	= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		// ViewPort
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = (float)WinNative::getInstance()->getWinSizeX();
		viewport.height	= (float)WinNative::getInstance()->getWinSizeY();
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;

		scissor.offset = { 0, 0 };
		scissor.extent = {WinNative::getInstance()->getWinSizeX(), WinNative::getInstance()->getWinSizeY()};

		viewportState.sType	= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount	= 1;
		viewportState.pViewports = &viewport;
		viewportState.scissorCount = 1;
		viewportState.pScissors = &scissor;

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		// Rasterizer
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.depthClampEnable	= VK_FALSE;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizer.lineWidth = 1.0f;
		rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterizer.cullMode	= VK_CULL_MODE_NONE;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasConstantFactor = 0.0f; // Optional
		rasterizer.depthBiasClamp = 0.0f; // Optional
		rasterizer.depthBiasSlopeFactor	= 0.0f; // Optional

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		// MSAA
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		multisampling.sType	= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisampling.minSampleShading = 1.0f; // Optional
		multisampling.pSampleMask = nullptr; /// Optional
		multisampling.alphaToCoverageEnable	= VK_FALSE; // Optional
		multisampling.alphaToOneEnable = VK_FALSE; // Optional

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		// Depth Stencil
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthTestEnable = VK_TRUE;
		depthStencil.depthWriteEnable = VK_TRUE;
		depthStencil.depthCompareOp	= VK_COMPARE_OP_LESS; // lower depth == closer
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.minDepthBounds = 0.f;
		depthStencil.maxDepthBounds = 1.0f;
		depthStencil.stencilTestEnable = VK_FALSE;
		depthStencil.front = {};
		depthStencil.back = {};

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		// Color Blending
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachment.blendEnable = VK_FALSE;

		colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlending.logicOpEnable	= VK_FALSE;
		colorBlending.logicOp = VK_LOGIC_OP_COPY;
		colorBlending.attachmentCount = 1;
		colorBlending.pAttachments = &colorBlendAttachment;
		colorBlending.blendConstants[0]	= 0.0f;
		colorBlending.blendConstants[1]	= 0.0f;
		colorBlending.blendConstants[2]	= 0.0f;
		colorBlending.blendConstants[3]	= 0.0f;

		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------

		// Dynamic State
		// From here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
		dynamicState.resize(2);
		dynamicState[0] = VK_DYNAMIC_STATE_VIEWPORT;
		dynamicState[1] = VK_DYNAMIC_STATE_SCISSOR;
		
		dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateInfo.dynamicStateCount = (uint32_t)dynamicState.size();
		dynamicStateInfo.pDynamicStates = dynamicState.data();
		// Till here ------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	}


	FixedPipelineCreationInfo::~FixedPipelineCreationInfo()
	{
	}

}
