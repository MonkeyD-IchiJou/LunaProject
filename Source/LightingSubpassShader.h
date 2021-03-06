#ifndef DIR_LIGHT_PASS_SHADER_H
#define DIR_LIGHT_PASS_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"
#include <glm/glm.hpp>
#include "FramePacket.h"

namespace luna
{
	class VulkanImageBufferObject;
	class UBO;

	class LightingSubpassShader :
		public ShaderProgram
	{
	public:
		LightingSubpassShader();
		virtual ~LightingSubpassShader();

		void Init(const VkRenderPass& renderpass, uint32_t subpassindex = 0) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;
		
		void SetDescriptors(
			const VulkanImageBufferObject* color0,
			const UBO* pointlights_ubo
		);

		void LoadPushConstantDatas(const VkCommandBuffer& commandbuffer, const MainDirLightData& dirlightdata, const glm::vec4& campos);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif

