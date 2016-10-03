#ifndef SIMPLE_SHADER_H
#define SIMPLE_SHADER_H

#include "ShaderProgram.h"

namespace luna
{
	class UBO;
	class SSBO;

	class SimpleShader :
		public ShaderProgram
	{
	public:
		SimpleShader();
		virtual ~SimpleShader();

		void Init(const VkRenderPass& renderpass) override;
		void Destroy() override;
		void Bind(const VkCommandBuffer& commandbuffer) override;

		// load push constant offset
		void LoadOffset(const VkCommandBuffer& commandbuffer, const int& offset);

		// REQUIREMENT: set the ubo buffers
		inline void setUBO(UBO* ubo) { m_ubo = ubo; }

		// REQUIREMENT: set the ssbo buffers
		inline void setSSBO(SSBO* ssbo) { m_ssbo = ssbo; }

		// if ssbo size is different, update the descriptor sets about it before binding 
		void RewriteSSBODescriptorSets();

	private:
		void CreatePipelineLayout_() override;
		void CreateDescriptorSets_();

	private:
		UBO* m_ubo = nullptr;
		SSBO* m_ssbo = nullptr;

		VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet m_descriptorSet = VK_NULL_HANDLE;
	};
}

#endif

