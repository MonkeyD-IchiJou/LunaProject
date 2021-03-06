#include "ShaderProgram.h"
#include "Renderer.h"
#include "DebugLog.h"

#include "IOManager.h"

namespace luna
{
	ShaderProgram::ShaderProgram()
	{
		/* get the handle for the logical device */
		m_logicaldevice = Renderer::getInstance()->GetLogicalDevice();
	}

	ShaderProgram::~ShaderProgram()
	{
	}

	void ShaderProgram::SetUpFixedPipeline_(FixedPipelineCreationTool & fixedpipeline)
	{
		// do nothing
	}

	void ShaderProgram::SetViewPort(const VkCommandBuffer& commandbuffer, const VkExtent2D& size, float mindepth, float maxdepth)
	{
		VkViewport viewport{};
		viewport.width = static_cast<float>(size.width);
		viewport.height = static_cast<float>(size.height);
		viewport.minDepth = mindepth;
		viewport.maxDepth = maxdepth;

		VkRect2D scissor{};
		scissor.extent = size;
		scissor.offset = { 0, 0 };

		vkCmdSetViewport(commandbuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandbuffer, 0, 1, &scissor);
	}

	VkPipelineShaderStageCreateInfo ShaderProgram::CreateShaders_(const std::string & shaderFileName)
	{
		/* get all the code from the shader */
		auto ShaderCode	= readShaderFile_(shaderFileName);

		/* create a shader module for it */
		VkShaderModule ShaderModule	= VK_NULL_HANDLE;
		CreateShaderModule_(ShaderCode, ShaderModule);

		VkPipelineShaderStageCreateInfo ShaderStageInfo{};
		ShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		ShaderStageInfo.module = ShaderModule;

		return ShaderStageInfo;
	}

	std::vector<char> ShaderProgram::readShaderFile_(const std::string & filename)
	{
		std::vector<char> buffer;
		IO::LoadFile(filename, buffer);
		return buffer;
	}

	void ShaderProgram::CreateShaderModule_(const std::vector<char>& code, VkShaderModule & shaderModule)
	{
		VkShaderModuleCreateInfo shader_create_info{};
		shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_create_info.codeSize	= code.size();
		shader_create_info.pCode = (uint32_t*)code.data();

		DebugLog::EC(vkCreateShaderModule(m_logicaldevice, &shader_create_info, nullptr, &shaderModule));
	}
}
