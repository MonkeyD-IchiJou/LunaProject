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
layout(location = 3) in vec4 inViewNormal;
layout(location = 4) in vec4 inWSNormal;
layout(location = 5) in vec4 inVPos;
layout(location = 6) in vec4 inVPrevPos;

// output as color attachment
layout (location = 0) out uvec4 color0;
layout (location = 1) out uvec4 color1;

void main()
{
	uvec4 outvec0 = uvec4(0);
	uvec4 outvec1 = uvec4(0);
	
	vec4 NormalizedViewNormal = inViewNormal;
	vec4 NormalizedWSNormal = inWSNormal;
	
	vec4 color = texture(samplerColor, vec2(NormalizedViewNormal.w, NormalizedWSNormal.w)) + vec4(inMaterialColor.xyz, 0.0);
	// inMaterialColor.a // material id
	
	outvec0.x = packUnorm4x8(color);
	outvec0.y = packHalf2x16(NormalizedWSNormal.xy);
	outvec0.z = packHalf2x16(vec2(NormalizedWSNormal.z, inWSPos.x));
	outvec0.w = packHalf2x16(inWSPos.yz);
	
	outvec1.x = packHalf2x16(inViewPos.xy);
	outvec1.y = packHalf2x16(vec2(inViewPos.z, NormalizedViewNormal.x));
	outvec1.z = packHalf2x16(NormalizedViewNormal.yz);
	
	// velocity buffer calc
	vec2 a = (inVPos.xy / inVPos.w) * 0.5 + 0.5;
    vec2 b = (inVPrevPos.xy / inVPrevPos.w) * 0.5 + 0.5;
	outvec1.w = packHalf2x16(a - b); // velocity
	
	color0 = outvec0;
	color1 = outvec1;
}