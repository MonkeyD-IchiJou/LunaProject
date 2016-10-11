#ifndef TEXT_SHADER_H
#define TEXT_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"

namespace luna
{
	class VulkanImageBufferObject;
	class SSBO;

	class TextShader :
		public ShaderProgram
	{
	public:
		TextShader();
		virtual ~TextShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		void SetDescriptors(const SSBO* ssbo, const VulkanImageBufferObject* font_image);
		void UpdateDescriptor(const SSBO* ssbo);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif
