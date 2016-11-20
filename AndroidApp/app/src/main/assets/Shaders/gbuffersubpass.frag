#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;

// textures sampler
layout (set = 1, binding = 0) uniform sampler2D samplerColor;

// input from vertex shader
layout(location = 0) in vec4 inViewPos;
layout(location = 1) in vec4 inWSPos;
layout(location = 2) in vec4 inMaterialColor;
layout(location = 3) in vec3 inViewNormal;
layout(location = 4) in vec3 inWSNormal;
layout(location = 5) in vec2 inUV;

// output as color attachment
layout (location = 0) out uvec4 color0;
layout (location = 1) out uvec4 color1;
layout (location = 2) out vec4 color2;

void main()
{
	uvec4 outvec0 = uvec4(0);
	uvec4 outvec1 = uvec4(0);
	
	vec4 color = texture(samplerColor, inUV) + vec4(inMaterialColor.xyz, 0.0);
	
	vec3 NormalizedViewNormal = normalize(inViewNormal);
	vec3 NormalizedWSNormal = normalize(inWSNormal);
	
	outvec0.x = packHalf2x16(color.xy);
	outvec0.y = packHalf2x16(vec2(color.z, NormalizedWSNormal.x));
	outvec0.z = packHalf2x16(NormalizedWSNormal.yz);
	outvec0.w = packHalf2x16(vec2(color.a, inMaterialColor.a)); // specular color and material id
	
	outvec1.x = packHalf2x16(inViewPos.xy);
	outvec1.y = packHalf2x16(vec2(inViewPos.z, NormalizedViewNormal.x));
	outvec1.z = packHalf2x16(NormalizedViewNormal.yz);
	outvec1.w = packHalf2x16(vec2(0.0, 0.0)); // depth && ??
	
	color0 = outvec0;
	color1 = outvec1;
	color2 = inWSPos;
}