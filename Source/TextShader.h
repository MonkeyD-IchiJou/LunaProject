#ifndef TEXT_SHADER_H
#define TEXT_SHADER_H

#include "ShaderProgram.h"

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
		void UpdateDescriptorSets(const SSBO* ssbo, const VulkanImageBufferObject* font_image);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline) override;
		void CreatePipelineLayout_() override;
		void CreateDescriptorSets_();

	private:
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
	};
}

#endif
