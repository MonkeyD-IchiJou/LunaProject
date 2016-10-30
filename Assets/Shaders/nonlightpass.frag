#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (input_attachment_index = 0, set = 0, binding = 0) uniform usubpassInput attachmentColor0;

// input from vertex shader
layout (location = 0) in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor; // output to hdr texture attachment

void main()
{
	// Get G-Buffer values
	uvec4 data0 = subpassLoad(attachmentColor0);
	
	vec2 temp = unpackHalf2x16(data0.y);
	vec3 diffusecolor = vec3(unpackHalf2x16(data0.x), temp.x);
	
	// output the final color
	outFragcolor = vec4(diffusecolor.xyz, 1.0);
}