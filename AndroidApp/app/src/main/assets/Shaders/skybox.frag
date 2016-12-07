#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;

// textures sampler
layout (set = 1, binding = 0) uniform samplerCube samplerCubeMap;

// input from vertex shader
layout(location = 0) in vec3 inUVW;

// output as hdr color
layout (location = 0) out vec4 outFragcolor; // output to hdr texture attachment
layout (location = 1) out uvec4 outdata;

void main()
{
	outFragcolor = texture(samplerCubeMap, inUVW);
	
	outdata = uvec4(0.0);
	
}