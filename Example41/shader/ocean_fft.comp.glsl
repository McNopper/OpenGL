#version 430 core

// see same define in main.c
#define N 128

uniform int u_xFactor;
uniform int u_yFactor;

uniform float u_deltaTime;

layout(std430, binding=1) buffer VertexIn {
    vec4 b_vertexIn[];
};

layout(std430, binding=2) buffer VertexOut {
    vec4 b_vertexOut[];
};

// as N = 128, so local size is 128/2 = 64
layout (local_size_x = 64) in;

void main(void)
{
	int evenIndex = 2 * int(gl_GlobalInvocationID.x) * u_xFactor + int(gl_GlobalInvocationID.y) * u_yFactor;
	int oddIndex = 2 * int(gl_GlobalInvocationID.x) * u_xFactor + int(gl_GlobalInvocationID.y) * u_yFactor + u_xFactor;

	//
	// Calculate vertices.
	//
	
	b_vertexOut[evenIndex] = b_vertexIn[evenIndex];
	b_vertexOut[oddIndex] = b_vertexIn[oddIndex];
}
