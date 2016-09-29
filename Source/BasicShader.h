#ifndef BASIC_SHADER_H
#define BASIC_SHADER_H

#include "ShaderProgram.h"
#include "vk_glm.h"

namespace luna
{
	class BasicUBO;
	class BasicImage;

	/* just a normal shader which take in vertex and fragment shaders only */
	class BasicShader :
		public ShaderProgram
	{
	public:
		BasicShader();
		virtual ~BasicShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		void LoadModelMatrix(const VkCommandBuffer& commandbuffer, const glm::mat4& mat);
		void LoadColor(const VkCommandBuffer& commandbuffer, const glm::vec4& color);
		inline void SetUBO(BasicUBO* ubo) { this->m_ubo = ubo; }
		inline void SetTexture(BasicImage* image) { this->m_image = image; }

	private:
		void CreatePipelineLayout_() override;
		void CreateDescriptorSets_();

	private:
		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
		BasicUBO* m_ubo = nullptr;
		BasicImage* m_image = nullptr;
	};
}

#endif
