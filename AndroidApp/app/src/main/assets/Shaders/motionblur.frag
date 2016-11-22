#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform usampler2D InputData; // velocity data
layout (binding = 1) uniform sampler2D InputColor; // color tex that need to be post-processed

// input from vertex shader
layout (location = 0) in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor; // output as hdr color

void main()
{
	int uvx = int(1920.0 * inUV.x);
	int uvy = int(1080.0 * inUV.y);
	
	uvec4 data = texelFetch(InputData, ivec2(uvx, uvy), 0);
	
	vec2 temp = unpackHalf2x16(data.y);
	vec3 viewpos = vec3(unpackHalf2x16(data.x), temp.x);
	vec3 normal = vec3(temp.y, unpackHalf2x16(data.z));
	vec2 velocity = unpackHalf2x16(data.w);
	
	vec3 color = texture(InputColor, inUV).rgb;
	
	outFragcolor = vec4(color, 1.0);
}