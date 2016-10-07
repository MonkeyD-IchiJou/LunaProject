#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D textColor;

// input from vertex shader
layout (location = 0) in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor;


void main()
{
	outFragcolor = texture(textColor, inUV);
	//outFragcolor = vec4(inUV, 0.0, 0.5);
}