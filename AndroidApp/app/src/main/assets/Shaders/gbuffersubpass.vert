#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

// input from vertex attribute
layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inTexCoord;

// output to fragment shader
layout(location = 0) out vec4 outViewPos;
layout(location = 1) out vec4 outWSPos;
layout(location = 2) out vec4 outMaterialColor;
layout(location = 3) out vec3 outViewNormal;
layout(location = 4) out vec3 outWSNormal;
layout(location = 5) out vec2 outUV;

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
	
	mat4 model = instance[index].model;
	mat4 modelview = ubo.view * model;
	
	outWSPos = model * vec4(inPosition, 1.0);
	outViewPos = modelview * vec4(inPosition, 1.0);
	gl_Position = ubo.proj * outViewPos;
	
	// out texcoord
	outUV = inTexCoord; 
	
	// normal in world space 
	outWSNormal = transpose(inverse(mat3(model))) * normalize(inNormal);
	
	// normal in view space
	outViewNormal = transpose(inverse(mat3(modelview))) * normalize(inNormal);

	// out material color
	outMaterialColor = instance[index].material;
}