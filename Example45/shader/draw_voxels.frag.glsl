#version 440 core

layout (r32ui, binding = 0) uniform uimage3D u_voxelGrid;

in vec2 v_texCoord;

layout (location = 0) out vec4 fragColor;

void main(void)
{
	ivec3 dim = imageSize(u_voxelGrid);
	
	// Sample point location and integer conversion results in range from 0 to length - 1.  
	ivec2 xy = ivec2(vec2(dim.xy) * v_texCoord); 

	// RGBA information is in the red unsigned integer channel.
	uint packedValue;
	
	// Final color.
	vec4 color;
	
	// Iterate from front to back.
	for (int z = 0; z < dim.z; z++)
	{
		// Retrieve packed color information and ...
		packedValue = imageLoad(u_voxelGrid, ivec3(xy, z)).r;
		
		// ... convert it back.
		color = unpackUnorm4x8(packedValue);
		
		// If the counter is larger than zero, a color is found ...
		if (color.a > 0.0)
		{
			// ... write it and ...
			fragColor = color;
			
			// ... break the loop and return.
			return;
		}
	}
	
	// No voxel information, so discard fragment.
	discard;
}
