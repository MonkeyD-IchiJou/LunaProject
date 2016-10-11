#ifndef DIR_LIGHT_PASS_SHADER_H
#define DIR_LIGHT_PASS_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"

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

		void SetDescriptors(const VulkanImageBufferObject* samplerPos, 
			const VulkanImageBufferObject* samplerNormal, const VulkanImageBufferObject* samplerAlbedo);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif

