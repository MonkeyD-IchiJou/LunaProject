#ifndef FIXED_PIPELINE_CREATION_INFO_H
#define FIXED_PIPELINE_CREATION_INFO_H

#include "platform.h"

#include <vector>
#include <string>

namespace luna
{
	/* all the fixed pipeline info is here, will be preinitialised */
	struct FixedPipelineCreationTool
	{
	public:
		VkVertexInputBindingDescription bindingDescription{};
		std::vector<VkVertexInputAttributeDescription> attributeDescription{};
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		VkViewport viewport{};
		VkRect2D scissor{};
		VkPipelineViewportStateCreateInfo viewportState{};
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		VkPipelineMultisampleStateCreateInfo multisampling{};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{}; // various fbo various color blend option
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		std::vector<VkDynamicState> dynamicState;

		FixedPipelineCreationTool();
		~FixedPipelineCreationTool();
	};
}

#endif

