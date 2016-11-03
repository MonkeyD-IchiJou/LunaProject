#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;

// textures sampler
layout (set = 1, binding = 0) uniform samplerCube samplerCubeMap;

// input from vertex shader
layout(location = 0) in vec3 inUVW;

// output as color attachment
layout (location = 0) out uvec4 color0;
layout (location = 1) out vec4 color1;

void main()
{
	uvec4 outvec0 = uvec4(0);
	vec4 outvec1 = vec4(0);
	
	vec4 color = texture(samplerCubeMap, inUVW);
	
	outvec0.x = packHalf2x16(color.xy);
	outvec0.y = packHalf2x16(vec2(color.z, inUVW.x));
	outvec0.z = packHalf2x16(inUVW.yz);
	outvec0.w = 0;
	
	outvec1.xyz = inUVW.xyz;
	outvec1.w = color.a;
	
	color0 = outvec0;
	color1 = outvec1;
}