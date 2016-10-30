#ifndef NON_LIGHT_PASS_SHADER
#define NON_LIGHT_PASS_SHADER

#include "ShaderProgram.h"
#include "DescriptorTool.h"

namespace luna
{
	class VulkanImageBufferObject;

	class NonLightPassShader :
		public ShaderProgram
	{
	public:
		NonLightPassShader();
		virtual ~NonLightPassShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		void SetDescriptors(const VulkanImageBufferObject* samplerAlbedo);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};

	};
}

#endif

