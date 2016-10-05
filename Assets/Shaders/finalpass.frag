#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D finalColor;

// input from vertex shader
layout (location = 0) in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor;


void main()
{
	// do some hdr and gamma correction
	
	outFragcolor = texture(finalColor, inUV);
}