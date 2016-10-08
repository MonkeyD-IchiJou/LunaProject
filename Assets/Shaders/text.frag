#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 1) uniform sampler2D textColor;

// input from vertex shader
layout (location = 0) in vec4 fontcolor;
layout (location = 1) in vec4 fontproperties;
layout (location = 2) in vec3 outlineColor;
layout (location = 3) in vec2 offset;
layout (location = 4) in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor;

//const vec4 fontcolor = vec4(1.0, 1.0, 1.0, 0.0);
// width, edge, borderwidth, borderedge
//const vec4 fontproperties = vec4(0.45, 0.15, 0.5, 0.15);
//const vec3 outlineColor = vec3(0.55, 0.23, 0.1);
//const vec2 offset = vec2(0.0015, 0.000);

void main()
{
	float width = fontproperties.x;
	float edge = fontproperties.y;
	float borderWidth = fontproperties.z;
	float borderEdge = fontproperties.w;
	
	float distance = 1.0 - texture(textColor, inUV).a;
	float alpha = 1.0 - smoothstep(width, width + edge, distance);
	
	float distance2 = 1.0 - texture(textColor, inUV + offset).a;
	float outlineAlpha = 1.0 - smoothstep(borderWidth, borderWidth + borderEdge, distance2);
	
	float overallAlpha = alpha + (1.0 - alpha) * outlineAlpha;
	vec3 overallColor = mix(outlineColor, fontcolor.rgb, alpha / overallAlpha);
	
	outFragcolor = vec4(overallColor, overallAlpha - fontcolor.a);
}