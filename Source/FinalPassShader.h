#ifndef FINAL_PASS_SHADER_H
#define FINAL_PASS_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"

namespace luna
{
	class VulkanImageBufferObject;

	class FinalPassShader :
		public ShaderProgram
	{
	public:
		FinalPassShader();
		virtual ~FinalPassShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		void SetDescriptors(const VulkanImageBufferObject* finalimage);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif