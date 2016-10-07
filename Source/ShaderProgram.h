#ifndef SHADER_PROGRAM_H
#define SHADER_PROGRAM_H

#include "FixedPipelineCreationInfo.h"
#include <mutex>

namespace luna
{
	class ShaderProgram
	{
	public:
		ShaderProgram();
		virtual ~ShaderProgram();

		/* init the graphics pipeline and necessary info */
		/* tell the shader which framebuffer renderpass it is bound to*/
		virtual void Init(const VkRenderPass& renderpass) = 0;

		/* destroy the graphics pipeline */
		virtual void Destroy() = 0;

		/* bind the shader graphics pipeline */
		/* bind the descriptor layout and descriptor sets if have any */
		virtual void Bind(const VkCommandBuffer& commandbuffer) = 0;

		/* set the dynamic view port */
		void SetViewPort(const VkCommandBuffer& commandbuffer, const VkExtent2D& size);

	protected:
		virtual void CreatePipelineLayout_() = 0;
		virtual void SetUpFixedPipeline_(FixedPipelineCreationTool& fixedpipeline);
		VkPipelineShaderStageCreateInfo CreateShaders_(const std::string& shaderFileName);
		
	protected:
		/* every shaders must have a graphic pipeline */
		VkPipeline m_graphicPipeline = VK_NULL_HANDLE;

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
