#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec2 inPosition;

// output datas to fragment
layout (location = 0) out vec4 fontcolor;
layout (location = 1) out vec4 fontproperties;
layout (location = 2) out vec3 outlineColor;
layout (location = 3) out vec2 offset;
layout (location = 4) out vec2 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
};

// instance data struct
struct InstanceData
{
	mat4 transformation;
	mat4 fontMaterials;
	vec2 uv[4];
};

// shader storage buffer object binding at 0
layout(std430, binding = 0) buffer sb
{
	InstanceData instance[];
};

void main()
{
	// position transform
	gl_Position = instance[gl_InstanceIndex].transformation * vec4(inPosition, 0.0, 1.0);
	gl_Position.z = 0.0f;
	
	// uv output to fragment
	outUV = instance[gl_InstanceIndex].uv[gl_VertexIndex];
	
	mat4 materials = instance[gl_InstanceIndex].fontMaterials;
	fontcolor = materials[0];
	fontproperties = materials[1];
	outlineColor = materials[2].xyz;
	offset = materials[3].xy;
}