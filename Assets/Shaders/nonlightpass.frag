#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput attachmentAlbedo;

// input from vertex shader
layout (location = 0) in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor; // output to hdr texture attachment

void main()
{
	// Get G-Buffer values
	vec4 albedo = subpassLoad(attachmentAlbedo);
	
	// output the final color
	outFragcolor = vec4(albedo.rgb, 1.0);
}