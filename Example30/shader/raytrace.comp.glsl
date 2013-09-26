#version 430 core

#define MAX_DEPTH 5

// see g_localSize = 16 in main.c. Also the division by 16.0 depends on this size.
layout (local_size_x = 16, local_size_y = 16) in;

layout(binding = 0, rgba8) uniform image2D u_texture; 

layout (binding = 1) buffer Directions
{
	vec3 direction[];
	// Padding
} b_directions;

layout (binding = 2) buffer Positions
{
	vec4 position[];
} b_positions;

struct Ray {
	vec4 position;
	vec3 direction;
	float valid;
};

layout (binding = 3) buffer RayStack
{
	Ray ray[];
} b_rayStack;

int getReflectIndex(int index)
{
	return index * 2 + 1;
}

int getRefractIndex(int index)
{
	return index * 2 + 2;
}

void trace(int rayIndex)
{
	if (b_rayStack.ray[rayIndex].valid <= 0.0)
	{
		return;
	}
	
	// TODO Do checks similar as in Example29

	// TODO If reflection:
	// b_rayStack.ray[getReflectIndex(rayIndex)].valid = 1.0;
	// Store position and direction
	
	// TODO If refraction:
	// b_rayStack.ray[getRefractIndex(rayIndex)].valid = 1.0;
	// Store position and direction
	
	// TODO Store hit sphere. 
}

void main(void)
{
	ivec2 dimension = imageSize(u_texture);

	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

	// Maximum possible traces	
	int maxLoops = int(exp2(MAX_DEPTH) - 1.0);

	// Init ray stack 0 with initial direction and position.
	b_rayStack.ray[0].position = b_positions.position[storePos.x + storePos.y * dimension.x];
	b_rayStack.ray[0].direction = b_directions.direction[storePos.x + storePos.y * dimension.x];

	// Trace all possible rays initiated by the origin ray.
	for (int i = 0; i < maxLoops; i++)
	{
		trace(i);
	}
	
	// TODO Loop again and calculate final color.

	//
	// TODO Temp: Printing out origin ray directions for debug purposes.
	//
		
	vec4 testColor = vec4(b_directions.direction[storePos.x + storePos.y * dimension.x], 1.0);
	
	imageStore(u_texture, storePos, testColor);	
}
