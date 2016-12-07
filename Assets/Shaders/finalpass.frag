#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform sampler2D hdrColor; // pls be hdr texture

// input from vertex shader
layout (location = 0) noperspective in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor; // output to screen (rgba8)

vec3 ToneMapFilmic_Hejl2015(vec3 hdr, float whitePt)
{
	vec4 vh = vec4(hdr, whitePt);
	vec4 va = (1.425 * vh) + 0.05;
	vec4 vf = ( (vh * va + 0.004) / ( (vh * (va + 0.55) + 0.0491) ) ) - 0.0821;
	return vf.rgb / vf.www;
}

/*
const float FXAA_SPAN_MAX = 8.0;
const float FXAA_REDUCE_MUL = 0.125; //1.0/8.0;
const float FXAA_REDUCE_MIN = 0.0078125; //1.0/128.0;
*/

void main()
{
	// tone mapping (hdr to ldr) here
	// Exposure tone mapping
	// tone mapping filmic stlye
	outFragcolor = vec4(ToneMapFilmic_Hejl2015(vec3(1.0) - exp(-texture(hdrColor, inUV).rgb * 1.0), 1.5), 1.0);
	
	/*
	
	// tone mapping + FXAA
	vec2 frameBufSize = vec2(textureSize(hdrColor, 0));
	
	vec3 rgbNW = texture(hdrColor,inUV+(vec2(-1.0,-1.0)/frameBufSize)).xyz;
    vec3 rgbNE = texture(hdrColor,inUV+(vec2(1.0,-1.0)/frameBufSize)).xyz;
    vec3 rgbSW = texture(hdrColor,inUV+(vec2(-1.0,1.0)/frameBufSize)).xyz;
    vec3 rgbSE = texture(hdrColor,inUV+(vec2(1.0,1.0)/frameBufSize)).xyz;
    vec3 rgbM = texture(hdrColor,inUV).xyz;

    vec3 luma=vec3(0.299, 0.587, 0.114);
    float lumaNW = dot(rgbNW, luma);
    float lumaNE = dot(rgbNE, luma);
    float lumaSW = dot(rgbSW, luma);
    float lumaSE = dot(rgbSE, luma);
    float lumaM  = dot(rgbM,  luma);

    float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
    float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));

    vec2 dir;
    dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
    dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));

    float dirReduce = max(
        (lumaNW + lumaNE + lumaSW + lumaSE) * (0.25 * FXAA_REDUCE_MUL),
        FXAA_REDUCE_MIN);

    float rcpDirMin = 1.0/(min(abs(dir.x), abs(dir.y)) + dirReduce);

    dir = min(vec2( FXAA_SPAN_MAX,  FXAA_SPAN_MAX),
          max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
          dir * rcpDirMin)) / frameBufSize;

    vec3 rgbA = 0.5 * (
        texture(hdrColor, inUV.xy + dir * (-0.166666667)).xyz +
        texture(hdrColor, inUV.xy + dir * (0.1666666667)).xyz);
    vec3 rgbB = rgbA * 0.5 + 0.25 * (
        texture(hdrColor, inUV.xy + dir * (-0.5)).xyz +
        texture(hdrColor, inUV.xy + dir * (0.5)).xyz);
    float lumaB = dot(rgbB, luma);

	vec4 outfxaacolor = vec4(0.0);
	
    if((lumaB < lumaMin) || (lumaB > lumaMax))
	{
        outfxaacolor.xyz=rgbA;
    }
	else
	{
        outfxaacolor.xyz=rgbB;
    }

	outfxaacolor.a = 1.0;
	
	// final tone mapping tone mapping
	outFragcolor = vec4(ToneMapFilmic_Hejl2015(vec3(1.0) - exp(-outfxaacolor.rgb * 1.0), 1.5), 1.0);
	
	*/
}