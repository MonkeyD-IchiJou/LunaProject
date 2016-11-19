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
	vec4 dirdifspec;
	vec4 dirambientlight;
	vec4 dirlightdir; // dirlightpos.w == total pointlight
	vec4 campos;
} pushconsts;

struct pointlight
{
	vec4 position;
	vec4 color;
};

// storage buffer object binding at 4
layout(std430, set = 0, binding = 4) buffer sb
{
	pointlight pointlightinfo[];
};

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
vec3 rimCalc(vec3 N, vec3 V)
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

vec3 diffuseCalc(vec3 lightDiff, float n_dot_l)
{
	return lightDiff *  n_dot_l;
}

vec3 specularCalc(vec3 lightSpecular, vec3 normal, vec3 lightDir, vec3 viewDir, 
					vec3 specularColor, float n_dot_l)
{
	vec3 specular = vec3(0.0);
	if(n_dot_l > 0.0)
	{
		specular = (lightSpecular * 
				pow( max(dot(normal, normalize(lightDir + viewDir)), 0.0), 120.0) * specularColor);
	}

	return specular;
}

vec3 dirlight_fragment(fragment_info_t fragment, vec3 viewDir)
{
	// rmb all calculation in world space
	vec3 lightDir = normalize(-pushconsts.dirlightdir.xyz);
	vec3 dirlightdiff = pushconsts.dirdifspec.xyz;
	float n_dot_l = max( dot(lightDir, fragment.wsnormal), 0.0);
	vec4 result = vec4(0.0);
	
	vec3 ambient = pushconsts.dirambientlight.xyz;
	vec3 diffuse = diffuseCalc(dirlightdiff, n_dot_l);
	vec3 specular = specularCalc(vec3(pushconsts.dirdifspec.w), fragment.wsnormal, 
					lightDir, viewDir, vec3(fragment.specularcolor), n_dot_l);
	
	return specular + diffuse + ambient;
}

vec3 pointlight_fragment(fragment_info_t fragment, vec3 viewDir)
{
	int i = 0;
	vec3 result = vec3(0.0);
	vec3 ld = vec3(0.0);
	vec3 lightDir = vec3(0.0);
	float n_dot_l = 0.0;
	float distance = 0.0;
	
	for(i = 0; i < pushconsts.dirlightdir.w; ++i)
	{
		// rmb all calculation in world space
		ld = pointlightinfo[i].position.xyz - fragment.wspos;
		lightDir = normalize(ld);
		n_dot_l = max( dot(lightDir, fragment.wsnormal), 0.0);
				
		// Attenuation
		distance = length(ld);
		
		result += (diffuseCalc(pointlightinfo[i].color.xyz, n_dot_l) + 
				specularCalc(vec3(0.75), fragment.wsnormal,lightDir, viewDir, vec3(fragment.specularcolor), n_dot_l)) * 
				1.0 / (1.0 +  0.35 * distance + 0.44 * (distance * distance));
	}
	
	return result;
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
	vec3 viewDir = normalize(pushconsts.campos.xyz - fragmentinfo.wspos);
	
	// do some lighting calculation
	vec3 dirlightcompute = dirlight_fragment(fragmentinfo, viewDir);
	vec3 pointlightcompute = pointlight_fragment(fragmentinfo, viewDir);
	
	//vec3 rim = vec3(0.0);
	//if(fragmentinfo.materialID == 0)
	//{
	//	  rim = rimCalc(fragmentinfo.normal, normalize(-fragmentinfo.viewpos)) * pushconsts.dirdifspec.xyz;
	//}
	
	outFragcolor = vec4(fragmentinfo.diffusecolor * (dirlightcompute + pointlightcompute), 1.0);
}