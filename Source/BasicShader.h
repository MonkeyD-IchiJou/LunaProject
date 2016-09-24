#ifndef BASIC_SHADER_H
#define BASIC_SHADER_H

#include "ShaderProgram.h"

namespace luna
{
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

	private:
		void CreatePipelineLayout_() override;
	};
}

#endif
