#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (input_attachment_index = 0, set = 0, binding = 0) uniform usubpassInput attachmentColor0;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput attachmentColor1;

// input from vertex shader
layout (location = 0) in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor; // output to hdr texture attachment

struct DirLight {
    vec3 direction;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};  

void main()
{
	// Get G-Buffer values
	uvec4 data0 = subpassLoad(attachmentColor0);
	vec4 data1 = subpassLoad(attachmentColor1);
	
	vec2 temp = unpackHalf2x16(data0.y);
	vec3 diffusecolor = vec3(unpackHalf2x16(data0.x), temp.x);
	vec3 N = normalize(vec3(temp.y, unpackHalf2x16(data0.z)));
	
	vec3 worldpos = data1.xyz;
	float specularpower = data1.w;
	
	// do some lighting calculation
	DirLight dirLight;
	dirLight.direction = vec3(1.0, 1.0, 0);
	dirLight.ambient = vec3(0.01, 0.01, 0.01);
	dirLight.diffuse = vec3(1.0, 1.0, 1.0);
	dirLight.specular = vec3(0.5, 0.5, 0.5);
	
	float diff = max(dot(N, dirLight.direction), 0.0);
	vec3 diffuse = diff * dirLight.diffuse;
	
	// output the final color
	outFragcolor = vec4((diffuse + dirLight.ambient) * diffusecolor , 1.0);
}