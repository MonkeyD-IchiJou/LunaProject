#include "ShaderProgram.h"
#include "Renderer.h"
#include "DebugLog.h"
#include <fstream>

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

	void ShaderProgram::SetViewPort(const VkCommandBuffer& commandbuffer, const uint32_t & width, const uint32_t & height)
	{
		VkViewport viewport{};
		viewport.width = (float)width;
		viewport.height = (float)height;
		viewport.minDepth = 0.f;
		viewport.maxDepth = 1.f;

		VkRect2D scissor{};
		scissor.extent = { width, height };
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
#if VK_USE_PLATFORM_WIN32_KHR
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			DebugLog::throwEx("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		// seek back to the beggining of the file and read all of the bytes at once
		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;

#endif
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
