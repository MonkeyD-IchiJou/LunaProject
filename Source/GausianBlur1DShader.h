#ifndef GAUSIANBLUR_SHADER_H
#define GAUSIANBLUR_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"

#include <glm\vec2.hpp>

namespace luna
{
	class VulkanImageBufferObject;

	class GausianBlur1DShader :
		public ShaderProgram
	{
	public:
		GausianBlur1DShader();
		virtual ~GausianBlur1DShader();

		void Init(const VkRenderPass& renderpass, uint32_t subpassindex = 0) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;
		void BindDescriptorSet(const VkCommandBuffer& commandbuffer, const int& whichset);

		void SetDescriptors(const VulkanImageBufferObject* horizontalImage, const VulkanImageBufferObject* VerticalImage);
		void LoadBlurDirection(const VkCommandBuffer& commandbuffer, const glm::ivec2 & blurdir);
		void LoadResolution(const VkCommandBuffer& commandbuffer, const glm::ivec2 & resolution);

	private:
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif
