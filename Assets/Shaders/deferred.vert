#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// input from vertex attribute
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// output to fragment shader
layout(location = 0) out vec4 outWorldPos;
layout(location = 1) out vec4 outMaterialColor;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec2 outUV;


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

// instance data struct
struct InstanceData
{
	mat4 model;
	mat4 transpose_inverse_model;
	vec4 material;
};

// shader storage buffer object binding at 1
layout(std430, set = 0, binding = 1) buffer sb
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
	int index = pushconsts.offset + gl_InstanceIndex;
	
	outWorldPos = instance[index].model * vec4(inPosition, 1.0);
	gl_Position = ubo.proj * ubo.view * outWorldPos;
	
	// out texcoord
	outUV = inTexCoord;
	
	// vertex pos in world space, make sure w is 1
	outWorldPos.w = 1; 
	
	// normal in world space
	outNormal = mat3(instance[index].transpose_inverse_model) * normalize(inNormal);

	// out material color
	outMaterialColor = instance[index].material;
}