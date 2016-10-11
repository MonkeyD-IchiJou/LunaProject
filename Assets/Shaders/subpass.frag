#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (input_attachment_index=0, set=0, binding = 0) uniform subpassInput attachmentColor;

// input from vertex shader
layout (location = 0) in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor;

void main()
{
	vec4 texcolor = subpassLoad(attachmentColor);
	outFragcolor = texcolor;
}