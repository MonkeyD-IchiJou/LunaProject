#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) out vec4 ocolor;

out gl_PerVertex
{
	vec4 gl_Position;
};

layout(binding = 0) uniform UniformBufferObject
{
	mat4 model;
	mat4 view;
	mat4 proj;
} ubo;

layout(push_constant) uniform PushConst
{
	mat4 model;
	vec4 color;
} pushconsts;

void main()
{
	gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 1.0);
	
	fragTexCoord = inTexCoord;
	ocolor = pushconsts.color;
}