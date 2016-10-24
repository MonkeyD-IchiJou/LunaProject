#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;

// textures sampler
layout (set = 1, binding = 0) uniform samplerCube samplerCubeMap;

// input from vertex shader
layout(location = 0) in vec3 inUVW;

// output as color attachment
layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

void main()
{
	outAlbedo = texture(samplerCubeMap, inUVW);
}