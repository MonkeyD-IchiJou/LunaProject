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
layout(location = 3) out vec4 outViewNormal;
layout(location = 4) out vec4 outWSNormal;
layout(location = 5) out vec4 outVPos;
layout(location = 6) out vec4 outVPrevPos;

out gl_PerVertex
{
	vec4 gl_Position;
};

// uniform buffer object binding at 0
layout(set = 0, binding = 0) uniform UniformBufferObject
{
	mat4 view;
	mat4 transpose_inverse_view;
	mat4 proj;
	mat4 prevprojview;
} ubo;

// instance data struct
struct InstanceData
{
	mat4 model;
	mat4 transpose_inverse_model;
	mat4 prevmodel;
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
	
	vec4 posL = vec4(inPosition, 1.0);
	
	outWSPos = instance[index].model * posL;
	outViewPos = ubo.view * instance[index].model * posL;
	outVPos = ubo.proj * outViewPos;
	outVPrevPos = ubo.prevprojview * instance[index].prevmodel * posL;
	
	// texcoord mix with the two normal
	
	// normal in world space 
	outWSNormal = vec4(mat3(instance[index].transpose_inverse_model) * inNormal, 
								inTexCoord.x);
	outWSNormal.xyz = normalize(outWSNormal.xyz);
	
	// normal in view space
	outViewNormal = vec4(mat3(ubo.transpose_inverse_view * instance[index].transpose_inverse_model) * inNormal, 
									inTexCoord.y);
	outViewNormal.xyz = normalize(outViewNormal.xyz);
	
	// out material color
	outMaterialColor = instance[index].material;
	
	gl_Position = outVPos;
}