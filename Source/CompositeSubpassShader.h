#ifndef COMPOSITE_SUBPASS_SHADER_H
#define COMPOSITE_SUBPASS_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"

namespace luna
{
	class VulkanImageBufferObject;

	class CompositeSubpassShader :
		public ShaderProgram
	{
	public:
		CompositeSubpassShader();
		virtual ~CompositeSubpassShader();

		void Init(const VkRenderPass& renderpass, uint32_t subpassindex = 0) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		void SetDescriptors(const VulkanImageBufferObject* comp1, const VulkanImageBufferObject* comp2);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif