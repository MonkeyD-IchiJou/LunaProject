#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D hdrColor; // pls be hdr texture

// input from vertex shader
layout (location = 0) in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor; // output to screen (rgba8)

vec3 ToneMapFilmic_Hejl2015(vec3 hdr, float whitePt)
{
	vec4 vh = vec4(hdr, whitePt);
	vec4 va = (1.425 * vh) + 0.05;
	vec4 vf = ( (vh * va + 0.004) / ( (vh * (va + 0.55) + 0.0491) ) ) - 0.0821;
	return vf.rgb / vf.www;
}

void main()
{
	// tone mapping (hdr to ldr) here
	// Exposure tone mapping
	// tone mapping filmic stlye
	outFragcolor = vec4(ToneMapFilmic_Hejl2015(vec3(1.0) - exp(-texture(hdrColor, inUV).rgb * 1.0), 1.5), 1.0);
	//outFragcolor = vec4(texture(hdrColor, inUV).rgb, 1.0);
}