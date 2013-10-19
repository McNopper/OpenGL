#version 430 core

layout (rgba8) uniform image2D u_texture; 

// see g_localSize = 16 in main.c. Also the division by 16.0 depends on this size.
layout (local_size_x = 16, local_size_y = 16) in;

void main(void)
{
	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

	// For every work group we make a color gradient effect ...
	vec4 color = vec4(float(gl_LocalInvocationID.x) / 16.0, float(gl_LocalInvocationID.y) / 16.0, 0.0, 1.0);

	// ... plus we make a checkerboard pattern out of it.
	if (gl_WorkGroupID.x % 2 + gl_WorkGroupID.y % 2 == 1)
	{
		color = vec4(0.0, 0.0, 0.0, 1.0);
	}

	imageStore(u_texture, storePos, color);
}
