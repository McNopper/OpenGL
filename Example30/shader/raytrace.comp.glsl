#version 430 core

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

// see g_localSize = 16 in main.c. Also the division by 16.0 depends on this size.
layout (local_size_x = 16, local_size_y = 16) in;

void main(void)
{
	ivec2 dimension = imageSize(u_texture);

	// TODO This is code from example 21, but the basics can be reused
	
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

	// For every work group we make a color gradient effect ...
	vec4 color = vec4(float(gl_LocalInvocationID.x) / 16.0, float(gl_LocalInvocationID.y) / 16.0, 0.0, 1.0);

	// ... plus we make a checkerboard pattern out of it.
	if (gl_WorkGroupID.x % 2 + gl_WorkGroupID.y % 2 == 1)
	{
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}

	// TODO Printing out content for debug purposes
	
	vec4 testColor = vec4(b_directions.direction[storePos.x + storePos.y * dimension.x], 1.0);
	
	imageStore(u_texture, storePos, testColor);	
}
