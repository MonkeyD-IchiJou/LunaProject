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
	vec3 hdrc = texture(hdrColor, inUV).rgb;
	
	 // Exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrc * 1.15); // at night is 0.5 .. else 1.75

	// tone mapping filmic stlye
	vec3 result = ToneMapFilmic_Hejl2015(mapped, 1.5);
	
	outFragcolor = vec4(result, 1.0);
}