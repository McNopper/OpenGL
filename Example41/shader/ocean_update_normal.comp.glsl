#version 430 core

// see same define in main.c
#define N 512
#define LENGTH	250.0

#define VERTEX_STEP (LENGTH / float(N - 1))
#define DIAGONAL_VERTEX_STEP sqrt(VERTEX_STEP * VERTEX_STEP * 2.0)

layout (binding = 0, rg32f) uniform image2D u_imageIn; 
layout (binding = 1, rgba32f) uniform image2D u_imageOut;

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

float sampleImage(ivec2 pos)
{
	ivec2 cpos = clamp(pos, 0, N - 1);
	
	return imageLoad(u_imageIn, cpos).r;
}

vec3 calculateNormal(ivec2 texCoord)
{
	vec3 normal;

	// Horizontal and vertical sampling.
	ivec2 right = ivec2(texCoord.s + 1, texCoord.t);
	ivec2 top = ivec2(texCoord.s, texCoord.t + 1);
	ivec2 left = ivec2(texCoord.s - 1, texCoord.t);
	ivec2 bottom = ivec2(texCoord.s, texCoord.t - 1);

	float slopeHorizontal = sampleImage(right) - sampleImage(left);
	float slopeVertical = sampleImage(top) - sampleImage(bottom);
	
	vec3 tangent = normalize(vec3(2.0 * VERTEX_STEP, slopeHorizontal, 0.0));
	vec3 bitangent = normalize(vec3(0.0, slopeVertical, -2.0 * VERTEX_STEP));
	
	normal += normalize(cross(tangent, bitangent));
	
	// Diagonal sampling.
	ivec2 rightTop = ivec2(texCoord.s + 1, texCoord.t + 1);
	ivec2 leftTop = ivec2(texCoord.s - 1, texCoord.t + 1);
	ivec2 leftBottom = ivec2(texCoord.s - 1, texCoord.t - 1);
	ivec2 rightBottom = ivec2(texCoord.s + 1, texCoord.t - 1);
	
	float slopeDown = sampleImage(rightBottom) - sampleImage(leftTop);
	float slopeUp = sampleImage(rightTop) - sampleImage(leftBottom);

	tangent = normalize(vec3(2.0 * DIAGONAL_VERTEX_STEP, slopeDown, 2.0 * DIAGONAL_VERTEX_STEP));
	bitangent = normalize(vec3(2.0 * DIAGONAL_VERTEX_STEP, slopeUp, -2.0 * DIAGONAL_VERTEX_STEP));

	normal += normalize(cross(tangent, bitangent));
		
	return normalize(normal);
}

void main(void)
{
	ivec2 storePos = ivec2(int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.y));

	vec3 normal = calculateNormal(storePos);

	imageStore(u_imageOut, storePos, vec4(normal, 0.0));
}
