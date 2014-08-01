#version 430 core

// see same define in main.c
#define N 512

uniform int u_processColumn;

layout (binding = 0, rg32f) uniform image2D u_imageIn; 
layout (binding = 1, rg32f) uniform image2D u_imageOut;

uniform float u_deltaTime;

// as N = 512, so local size is 512/2 = 256
layout (local_size_x = 256) in;

void main(void)
{
	ivec2 evenStorePos;
	ivec2 oddStorePos;
	
	if (u_processColumn == 0)
	{
		evenStorePos = ivec2(2 * int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.y));
		oddStorePos = ivec2(2 * int(gl_GlobalInvocationID.x) + 1, int(gl_GlobalInvocationID.y));
	}
	else
	{
		evenStorePos = ivec2(int(gl_GlobalInvocationID.y), 2 * int(gl_GlobalInvocationID.x));
		oddStorePos = ivec2(int(gl_GlobalInvocationID.y), 2 * int(gl_GlobalInvocationID.x) + 1);
	}

	// TODO Do inverse FFT.

	vec2 evenValue = vec2(1.0, 0.0);
	vec2 oddValue = vec2(1.0, 0.0);

	imageStore(u_imageOut, evenStorePos, vec4(evenValue, 0.0, 0.0));
	imageStore(u_imageOut, oddStorePos, vec4(oddValue, 0.0, 0.0));
}
