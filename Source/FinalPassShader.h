#ifndef FINAL_PASS_SHADER_H
#define FINAL_PASS_SHADER_H

#include "ShaderProgram.h"

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
		void UpdateDescriptorSets(const VulkanImageBufferObject* finalimage);

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