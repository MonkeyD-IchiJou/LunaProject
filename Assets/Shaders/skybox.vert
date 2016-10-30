#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// input from vertex attribute
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal; // not used
layout(location = 2) in vec2 inTexCoord; // not used

// output to fragment shader
layout(location = 0) out vec3 outUVW;

out gl_PerVertex
{
	vec4 gl_Position;
};

// uniform buffer object binding at 0
layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
} ubo;

layout(push_constant) uniform PushConst
{
	mat4 model;
} pushconsts;

void main()
{
	mat4 view = ubo.view;
	view[3][0] = 0.0;
	view[3][1] = 0.0;
	view[3][2] = 0.0;
	
	gl_Position = ubo.proj * view * pushconsts.model * vec4(inPosition, 1.0);
	
	// out texcoord
	outUVW = inPosition;
}