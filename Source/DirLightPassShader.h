#ifndef DIR_LIGHT_PASS_SHADER_H
#define DIR_LIGHT_PASS_SHADER_H

#include "ShaderProgram.h"

namespace luna
{
	class VulkanImageBufferObject;

	class DirLightPassShader :
		public ShaderProgram
	{
	public:
		DirLightPassShader();
		virtual ~DirLightPassShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;
		void UpdateDescriptorSets(const VulkanImageBufferObject* samplerPos, 
			const VulkanImageBufferObject* samplerNormal, const VulkanImageBufferObject* samplerAlbedo);

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

