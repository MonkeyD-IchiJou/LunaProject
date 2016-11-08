#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (input_attachment_index = 0, set = 0, binding = 0) uniform usubpassInput attachmentColor0;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput attachmentColor1;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput attachmentColor2;

// output as hdr color
layout (location = 0) out vec4 outFragcolor; // output to hdr texture attachment
 
layout(push_constant) uniform PushConst
{
	vec3 dirlightpos;
} pushconsts;

struct pointlight
{
	vec3 position;
	vec3 color;
};

// uniform buffer object binding at 3
layout(set = 0, binding = 3) uniform UniformBufferObject
{
	pointlight lightinfo[10];
} ubo_pointlights;

struct fragment_info_t
{
	vec3 diffusecolor;
	vec3 normal;
	vec3 viewpos;
	vec3 wspos;
	float specularcolor;
	float materialID;
};

const float rim_power = 7.0;

vec3 calculate_rim(vec3 N, vec3 V)
{
	// Calculate the rim factor
	float f = 1.0 - max(dot(N, V), 0.0);
	
	// Constrain it to the range 0 to 1 using a smoothstep function
	f = smoothstep(0.0, 1.0, f);

	// Raise it to the rim exponent
	f = pow(f, rim_power);
	
	// Finally, multiply it by the rim color
	return vec3(f);
}

vec4 pointlight_fragment(fragment_info_t fragment)
{
	vec4 result = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 lightcolor = ubo_pointlights.lightinfo[0].color;
	vec3 lightpos = ubo_pointlights.lightinfo[0].position;
	
	vec3 L = fragment.wspos - lightpos;
	float dist = length(L);
	L = normalize(L);
	vec3 N = normalize(fragment.normal);
	vec3 R = reflect(-L, N);
	float NdotR = max(0.0, dot(N, R));
	float NdotL = max(0.0, dot(N, L));
	float attenuation = 1.0 / (1.0 + 0.09 * dist + 0.032 * (dist * dist));
	
	vec3 diffuse_color = lightcolor *
						fragment.diffusecolor *
						NdotL * attenuation;
						
	vec3 specular_color = lightcolor *
						pow(NdotR, 128) *
						fragment.specularcolor *
						attenuation;
						
	result += vec4(diffuse_color + specular_color, 0.0);
	
	return result;
}

void unpackGBuffer(out fragment_info_t fragment)
{
	// Get G-Buffer values
	uvec4 data0 = subpassLoad(attachmentColor0);
	vec4 data1 = subpassLoad(attachmentColor1);
	vec4 data2 = subpassLoad(attachmentColor2);
	
	vec2 temp = unpackHalf2x16(data0.y);
	fragment.diffusecolor = vec3(unpackHalf2x16(data0.x), temp.x);
	fragment.normal = vec3(temp.y, unpackHalf2x16(data0.z));
	fragment.materialID = data0.w;
	
	fragment.viewpos = data1.xyz;
	fragment.specularcolor = data1.w;
	
	fragment.wspos = data2.xyz;
	
}
 
void main()
{
	fragment_info_t fragmentinfo;
	
	unpackGBuffer(fragmentinfo);
	
	// do some lighting calculation
	
	// calculate view-space light vector
	vec3 L = normalize(pushconsts.dirlightpos - fragmentinfo.viewpos);
	
	// calculate the view vector
	vec3 V = normalize(-fragmentinfo.viewpos);
	
	// calculate the half vector, H
	vec3 H = normalize(L + V);

	// Calculate the diffuse and specular contributions
	vec3 diffuse = max(dot(fragmentinfo.normal, L), 0.0) * 
					fragmentinfo.diffusecolor;
	float specular = pow(max(dot(fragmentinfo.normal, H), 0.0), 128.0) * 
						fragmentinfo.specularcolor;

	vec3 rim = vec3(0.0);
	if(fragmentinfo.materialID == 0)
	{
		rim = calculate_rim(fragmentinfo.normal, V);
	}
	
	// output the final color
	vec3 ambient = vec3(0.01, 0.01, 0.01);
	outFragcolor = vec4(rim + ambient + diffuse + specular, 1.0);
}