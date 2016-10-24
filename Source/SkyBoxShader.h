#ifndef SKYBOX_SHADER_H
#define SKYBOX_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"
#include <glm\glm.hpp>

namespace luna
{
	class UBO;
	class VulkanImageBufferObject;

	class SkyBoxShader :
		public ShaderProgram
	{
	public:
		SkyBoxShader();
		virtual ~SkyBoxShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		void SetDescriptors(const UBO* ubo, const VulkanImageBufferObject* cubemapimage);

		void LoadModel(const VkCommandBuffer& cmdbuff, const glm::mat4& model);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif // SKYBOX_SHADER_H
