#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;
//layout(location = 3) in vec3 inTangent;
//layout(location = 4) in vec3 inBitangent;

// output texcoord to fragment shader
layout(location = 0) out vec2 fragTexCoord;

out gl_PerVertex
{
	vec4 gl_Position;
};

// uniform buffer object binding at 0
layout(binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 proj;
	// mat4 prev_view;
	// mat4 inverse_view;
	// mat4 inverse_proj;
	// mat4 transpose_view;
} ubo;

// instance data struct
struct InstanceData
{
	mat4 model;
	// mat4 prev_model;
};

// shader storage buffer object binding at 1
layout(std430, binding = 1) buffer sb
{
	InstanceData instance[];
};

// offset telling shader where to start indexing the instance data
layout(push_constant) uniform PushConst
{
	int offset;
} pushconsts;

void main()
{
	gl_Position = ubo.proj * ubo.view * instance[pushconsts.offset + gl_InstanceIndex].model * vec4(inPosition, 1.0);
	
	fragTexCoord = inTexCoord;
}