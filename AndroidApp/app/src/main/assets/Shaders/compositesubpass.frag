#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (input_attachment_index = 0, set = 0, binding = 0) uniform subpassInput attachmentColor0;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput attachmentColor1;

// output as hdr color
layout (location = 0) out vec4 outFragcolor; // output to hdr texture attachment
 
void main()
{
	vec3 lightcolor = subpassLoad(attachmentColor0).xyz;
	vec3 nonlightcolor = subpassLoad(attachmentColor1).xyz;

	outFragcolor = vec4(lightcolor + nonlightcolor, 1.0);
}