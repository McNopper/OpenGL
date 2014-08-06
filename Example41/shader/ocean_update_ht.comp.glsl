#version 430 core

// see same define in main.c
#define N 512
#define LENGTH	250.0
#define GRAVITY 9.81
#define GLUS_PI	3.1415926535897932384626433832795

layout (binding = 0, rg32f) uniform image2D u_imageIn; 
layout (binding = 1, rg32f) uniform image2D u_imageOut;

uniform float u_totalTime;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

void main(void)
{
	ivec2 storePos = ivec2(int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.y));
	ivec2 storePos_negative = ivec2(N - 1 - storePos.x, N - 1 - storePos.y);

	vec2 h0 = imageLoad(u_imageIn, storePos).xy;
	vec2 h0_negative = imageLoad(u_imageIn, storePos_negative).xy;

	vec2 waveDirection;
	waveDirection.x = (float(-N) / 2.0 + gl_GlobalInvocationID.x) * (2.0 * GLUS_PI / LENGTH);
    waveDirection.y = (float(N) / 2.0 - gl_GlobalInvocationID.y) * (2.0 * GLUS_PI / LENGTH);

	// Note: To improve performance, put w2k in a look up texture.
	float w2k = GRAVITY * length(waveDirection);

	float wktime = sqrt(w2k) * u_totalTime;

	float cos_wktime = cos(wktime);
	float sin_wktime = sin(wktime);

	vec2 ht;
	ht.x = (h0.x * cos_wktime - h0.y * sin_wktime) + (h0_negative.x * cos_wktime - h0_negative.y * sin_wktime); 
	ht.y = (h0.x * sin_wktime + h0.y * cos_wktime) + (h0_negative.x * sin_wktime + h0_negative.y * cos_wktime); 

	imageStore(u_imageOut, storePos, vec4(ht, 0.0, 0.0));
}
