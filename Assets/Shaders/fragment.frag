#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in vec2 fragTexCoord;

void main()
{
	outColor = vec4(0.25, 0.5, 0.8, 1.0);

}