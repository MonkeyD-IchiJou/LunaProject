#ifndef MOTIONBLUR_PASS_SHADER_H
#define MOTIONBLUR_PASS_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"

namespace luna
{
	class VulkanImageBufferObject;

	class MotionBlurShader :
		public ShaderProgram
	{
	public:
		MotionBlurShader();
		virtual ~MotionBlurShader();

		void Init(const VkRenderPass& renderpass, uint32_t subpassindex = 0) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		void SetDescriptors(const VulkanImageBufferObject* dataimage, const VulkanImageBufferObject* colorimage);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif