#ifndef DEFERRED_SHADER_H
#define DEFERRED_SHADER_H

#include "ShaderProgram.h"
#include "DescriptorTool.h"
#include <glm\glm.hpp>

namespace luna
{
	class UBO;
	class SSBO;
	class VulkanImageBufferObject;

	class DeferredShader :
		public ShaderProgram
	{
	public:
		DeferredShader();
		virtual ~DeferredShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		void BindTexture(const VkCommandBuffer& commandbuffer, const int& whichset);

		void SetDescriptors(const UBO* ubo, const SSBO* ssbo, const std::vector<VulkanImageBufferObject*>& diffuseTexs);

		// if ssbo size is different, update the descriptor sets about it before binding 
		void UpdateDescriptor(const SSBO* ssbo);

		// load push constant offset
		void LoadObjectOffset(const VkCommandBuffer& commandbuffer, const int& offset);

		// load material color 
		void LoadObjectColor(const VkCommandBuffer& commandbuffer, const glm::vec4& color);

	private:
		void SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline) override;
		void CreatePipelineLayout_() override;

	private:
		DescriptorTool m_descriptorTool{};
	};
}

#endif

