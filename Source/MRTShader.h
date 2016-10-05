#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H

#include "ShaderProgram.h"

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
		void UpdateDescriptorSets(const UBO* ubo, const SSBO* ssbo, const VulkanImageBufferObject* image);

		// if ssbo size is different, update the descriptor sets about it before binding 
		void RewriteSSBODescriptorSets(const SSBO* ssbo);

		// load push constant offset
		void LoadObjectOffset(const VkCommandBuffer& commandbuffer, const int& offset);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline) override;
		void CreatePipelineLayout_() override;
		void CreateDescriptorSets_();

	private:
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
	};
}

#endif

