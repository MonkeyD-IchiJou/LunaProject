#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "FixedPipelineCreationInfo.h"

namespace luna
{
	class ShaderProgram
	{
	public:
		ShaderProgram();
		virtual ~ShaderProgram();

		/* init the graphics pipeline and necessary info */
		/* tell the shader which framebuffer renderpass it is bound to*/
		virtual void Init(const VkRenderPass& renderpass, uint32_t subpassindex = 0) = 0;

		/* destroy the graphics pipeline */
		virtual void Destroy() = 0;

		/* bind the shader graphics pipeline */
		/* bind the descriptor layout and descriptor sets if have any */
		virtual void Bind(const VkCommandBuffer& commandbuffer) = 0;

		/* set the dynamic view port */
		void SetViewPort(const VkCommandBuffer& commandbuffer, const VkExtent2D& size, float mindepth = 0.f, float maxdepth = 1.f);

	protected:
		virtual void CreatePipelineLayout_() = 0;
		virtual void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline);
		VkPipelineShaderStageCreateInfo CreateShaders_(const std::string& shaderFileName);
		
	protected:
		/* every shaders must have a graphic/compute pipeline */
		VkPipeline m_Pipeline = VK_NULL_HANDLE;

		/* every shaders must have a graphic pipeline layout */
		VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;

		/* logical device handle from the renderer */
		VkDevice m_logicaldevice;

	private:
		static std::vector<char> readShaderFile_(const std::string& filename);
		void CreateShaderModule_(const std::vector<char>& code, VkShaderModule& shaderModule);
	};
}

#endif
