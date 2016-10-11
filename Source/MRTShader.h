#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"

namespace luna
{
	class UBO;
	class SSBO;
	class VulkanImageBufferObject;

	class MRTShader :
		public ShaderProgram
	{
	public:
		MRTShader();
		virtual ~MRTShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		void SetDescriptors(const UBO* ubo, const SSBO* ssbo, const VulkanImageBufferObject* image);

		// if ssbo size is different, update the descriptor sets about it before binding 
		void UpdateDescriptor(const SSBO* ssbo);

		// load push constant offset
		void LoadObjectOffset(const VkCommandBuffer& commandbuffer, const int& offset);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif

