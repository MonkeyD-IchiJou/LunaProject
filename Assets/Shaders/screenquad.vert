#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec4 inPositionUV;

// output uv to fragment
layout (location = 0) noperspective out vec2 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main()
{
	gl_Position = vec4(inPositionUV.xy, 1.0, 1.0);
	outUV = inPositionUV.zw;
}