#ifndef COMPUTE_SHADER_H
#define COMPUTE_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"

namespace luna
{
	class VulkanImageBufferObject;

	class ComputeShader :
		public ShaderProgram
	{
	public:
		ComputeShader();
		virtual ~ComputeShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		void SetDescriptors(const VulkanImageBufferObject* readimage, const VulkanImageBufferObject* writeimage);

	private:
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif
