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
		VkViewport viewport{};
		VkRect2D scissor{};
		VkVertexInputBindingDescription bindingDescription{};
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		VkPipelineViewportStateCreateInfo viewportState{};
		VkPipelineRasterizationStateCreateInfo rasterizer{};
		VkPipelineMultisampleStateCreateInfo multisampling{};
		VkPipelineColorBlendStateCreateInfo colorBlending{};
		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		VkPipelineDynamicStateCreateInfo dynamicStateInfo{};
		std::vector<VkDynamicState> dynamicState;
		std::vector<VkVertexInputAttributeDescription> attributeDescription{};
		std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments{}; // various fbo various color blend option

		FixedPipelineCreationTool();
		~FixedPipelineCreationTool();
	};
}

#endif

