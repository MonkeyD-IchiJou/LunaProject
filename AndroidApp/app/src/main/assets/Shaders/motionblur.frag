#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (binding = 0) uniform usampler2D InputData; // velocity data
layout (binding = 1) uniform sampler2D InputColor; // color tex that need to be post-processed

// input from vertex shader
layout (location = 0) in vec2 inUV;

// output as color
layout (location = 0) out vec4 outFragcolor; // output as hdr color

const float uVelocityScale = 1.2; // currentfps / targetfps
const int MAX_SAMPLES = 64;

void main()
{
	vec2 actualSize = vec2(textureSize(InputData, 0));
	vec2 texelSize = 1.0 / actualSize;
	vec2 screenTexCoords = gl_FragCoord.xy * texelSize;
	
	uvec4 data = texelFetch(InputData, ivec2(int(actualSize.x * screenTexCoords.x), int(actualSize.y * screenTexCoords.y)), 0);
	vec2 temp = unpackHalf2x16(data.y);
	vec3 viewpos = vec3(unpackHalf2x16(data.x), temp.x);
	vec3 normal = vec3(temp.y, unpackHalf2x16(data.z));
	vec2 velocity = unpackHalf2x16(data.w);
	
	velocity *= uVelocityScale;
	
	// blur code will go here
	
	// how many samples going to take for the blur
	float speed = length(velocity / texelSize);
	int nSamples = clamp (int (speed), 1, MAX_SAMPLES);
	
	// actual blur
	outFragcolor = texture(InputColor, screenTexCoords);
	
	for (int i = 1; i < nSamples; ++i) 
	{
      vec2 offset = velocity * (float(i) / float(nSamples - 1) - 0.5);
      outFragcolor += texture(InputColor, screenTexCoords + offset);
	}
	
	outFragcolor /= float(nSamples);
	outFragcolor.a = 1.0;
}