#ifndef DIR_LIGHT_PASS_SHADER_H
#define DIR_LIGHT_PASS_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"
#include <glm/glm.hpp>

namespace luna
{
	class VulkanImageBufferObject;
	class UBO;

	class DirLightPassShader :
		public ShaderProgram
	{
	public:
		DirLightPassShader();
		virtual ~DirLightPassShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;
		void LoadDirLightPos(const VkCommandBuffer& commandbuffer, const glm::vec3& pos);

		void SetDescriptors(
			const VulkanImageBufferObject* color0,
			const VulkanImageBufferObject* color1,
			const VulkanImageBufferObject* color2,
			const VulkanImageBufferObject* color3,
			const UBO* ubo_pointlights
		);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif

