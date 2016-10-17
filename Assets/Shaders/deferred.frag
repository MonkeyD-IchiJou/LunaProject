#version 450
#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(early_fragment_tests) in;

// textures sampler
layout (binding = 2) uniform sampler2D samplerColor;

// input from vertex shader
layout(location = 0) in vec4 inWorldPos;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec2 inUV;

// output as color attachment
layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

float LinearizeDepth(float depth)
{
  float n = 0.1; // camera z near
  float f = 10.0; // camera z far
  float z = depth;
  return (2.0 * n) / (f + n - z * (f - n));	
}

void main()
{
	outPosition = inWorldPos;
	outNormal = vec4(inNormal, 1.0);
	outAlbedo = texture(samplerColor, inUV);
	//outAlbedo = vec4(1.0 - LinearizeDepth(gl_FragCoord.z));
}