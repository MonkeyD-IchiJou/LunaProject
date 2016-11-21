#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (input_attachment_index = 0, set = 0, binding = 0) uniform usubpassInput attachmentColor0;
layout (input_attachment_index = 1, set = 0, binding = 1) uniform usubpassInput attachmentColor1;

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

// uniform buffer object binding at 2
layout(set = 0, binding = 2) uniform UniformBufferObject
{
	pointlight pointlights[100]; // max 100 pointlights available
} ubo_pls;

struct fragment_info_t
{
	vec3 diffusecolor;
	vec3 normal;
	vec3 wsnormal;
	vec3 viewpos;
	vec3 wspos;
	vec2 velocity;
	float specularcolor;
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
		ld = ubo_pls.pointlights[i].position.xyz - fragment.wspos;
		lightDir = normalize(ld);
		n_dot_l = max( dot(lightDir, fragment.wsnormal), 0.0);
				
		// Attenuation
		distance = length(ld);
		
		result += (diffuseCalc(ubo_pls.pointlights[i].color.xyz, n_dot_l) + 
				specularCalc(vec3(0.75), fragment.wsnormal,lightDir, viewDir, vec3(fragment.specularcolor), n_dot_l)) * 
				1.0 / (1.0 +  0.35 * distance + 0.44 * (distance * distance));
	}
	
	return result;
}

void unpackGBuffer(out fragment_info_t fragment)
{
	// Get G-Buffer values
	uvec4 data0 = subpassLoad(attachmentColor0);
	uvec4 data1 = subpassLoad(attachmentColor1);
	
	vec4 temp_albedo = unpackUnorm4x8(data0.x);
	fragment.diffusecolor = temp_albedo.rgb;
	fragment.specularcolor = temp_albedo.a;
	
	vec2 temp = unpackHalf2x16(data0.z);
	fragment.wsnormal = vec3(unpackHalf2x16(data0.y), temp.x);
	fragment.wspos = vec3(temp.y, unpackHalf2x16(data0.w));
	
	temp = unpackHalf2x16(data1.y);
	fragment.viewpos = vec3(unpackHalf2x16(data1.x), temp.x);
	fragment.normal = vec3(temp.y, unpackHalf2x16(data1.z));
	fragment.velocity = unpackHalf2x16(data1.w);
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