#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (input_attachment_index = 0, set = 0, binding = 0) uniform usubpassInput attachmentColor0;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform subpassInput attachmentColor1;
layout (input_attachment_index = 2, set = 0, binding = 2) uniform subpassInput attachmentColor2;
layout (input_attachment_index = 3, set = 0, binding = 3) uniform subpassInput attachmentColor3;

// output as hdr color
layout (location = 0) out vec4 outFragcolor; // output to hdr texture attachment
 
layout(push_constant) uniform PushConst
{
	vec3 dirlightpos;
} pushconsts;

struct pointlight
{
	vec4 position;
	vec4 color;
};

// uniform buffer object binding at 4
layout(set = 0, binding = 4) uniform UniformBufferObject
{
	pointlight lightinfo[10];
} ubo_pointlights;

struct fragment_info_t
{
	vec3 diffusecolor;
	vec3 normal;
	vec3 wsnormal;
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

const vec3 lightdiff = vec3(0.5, 0.5, 0.5);
const float lightspec = 0.5;
const vec3 lightambient = vec3(0.01, 0.01, 0.01);
const float lightconstant = 1.0;
const float lightlinear = 0.22;
const float lightquadratic = 0.20;

vec4 pointlight_fragment(fragment_info_t fragment)
{
	vec3 norm = fragment.wsnormal;
	vec3 lightpos = vec3(0.0);
	vec3 lightcolor = vec3(0.0);
	vec3 finaloutput = vec3(0.0);
	float distance = 0.0;
	float attenuation = 0.0;
	vec3 ambient = vec3(0.0);
	vec3 diffuse = vec3(0.0);
	vec3 V = vec3(0.0);
	int i = 0;
	
	for(i = 0; i < 10; ++i)
	{
		lightpos = ubo_pointlights.lightinfo[i].position.xyz;
		lightcolor = ubo_pointlights.lightinfo[i].color.xyz;
		V = lightpos - fragment.wspos;
		
		// Attenuation
		distance = length(V);
		attenuation = 1.0 / (lightconstant + lightlinear * distance + lightquadratic * (distance * distance));
		
		// ambient
		ambient = lightambient * attenuation;
		
		// Diffuse
		diffuse = max(dot(norm, normalize(V)), 0.0) * 
					fragment.diffusecolor * lightcolor * attenuation;
		
		finaloutput += ambient + diffuse;
	}
	
	return vec4(finaloutput, 1.0);
}

vec4 dirlight_fragment(fragment_info_t fragment)
{
	// calculate view-space light vector
	vec3 L = normalize(pushconsts.dirlightpos - fragment.viewpos);
	
	// calculate the view vector
	vec3 V = normalize(-fragment.viewpos);
	
	// calculate the half vector, H
	vec3 H = normalize(L + V);

	// Calculate the diffuse and specular contributions
	vec3 diffuse = max(dot(fragment.normal, L), 0.0) * 
					fragment.diffusecolor * lightdiff;
	float specular = pow(max(dot(fragment.normal, H), 0.0), 128.0) * 
						fragment.specularcolor * lightspec;

	vec3 rim = vec3(0.0);
	if(fragment.materialID == 0)
	{
		rim = calculate_rim(fragment.normal, V) * lightdiff;
	}
	
	// output the final color
	vec3 ambient = lightambient;
	return vec4(rim + ambient + diffuse + specular, 1.0);
}

void unpackGBuffer(out fragment_info_t fragment)
{
	// Get G-Buffer values
	uvec4 data0 = subpassLoad(attachmentColor0);
	vec4 data1 = subpassLoad(attachmentColor1);
	vec4 data2 = subpassLoad(attachmentColor2);
	vec4 data3 = subpassLoad(attachmentColor3);
	
	vec2 temp = unpackHalf2x16(data0.y);
	fragment.diffusecolor = vec3(unpackHalf2x16(data0.x), temp.x);
	fragment.wsnormal = vec3(temp.y, unpackHalf2x16(data0.z));
	fragment.materialID = data0.w;
	
	fragment.viewpos = data1.xyz;
	fragment.specularcolor = data1.w;
	
	fragment.wspos = data2.xyz;
	
	fragment.normal = data3.xyz;
}
 
void main()
{
	fragment_info_t fragmentinfo;
	
	unpackGBuffer(fragmentinfo);
	
	// do some lighting calculation
	vec4 dirlightcompute = dirlight_fragment(fragmentinfo);
	vec4 pointlightcompute = pointlight_fragment(fragmentinfo);
	
	outFragcolor = vec4(dirlightcompute.xyz + pointlightcompute.xyz, 1.0);
}