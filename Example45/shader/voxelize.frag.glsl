#version 440 core

// As 1 byte for the counter.
#define MAX_COLOR_VALUES 256.0

layout (r32ui, binding = 0) uniform coherent volatile uimage3D u_voxelGrid;

layout (binding = 1) uniform sampler2D u_diffuse;

in vec4 v_aabb;
in vec2 v_texCoord;
in flat float v_orientation;

// see "Octree-Based Sparse Voxelization Using the GPU Hardware Rasterizer" in OpenGL Insights
void imageAtomicAverage(ivec3 pos, vec4 addingColor)
{
    // New value to store in the image.
    uint newValue = packUnorm4x8(addingColor);
    // Expected value, that data can be stored in the image.
    uint expectedValue = 0;
    // Actual data in the image.
    uint actualValue;

    // Average/sum of the voxel.
    vec4 color; 
    
    // Try to store ...
    actualValue = imageAtomicCompSwap(u_voxelGrid, pos, expectedValue, newValue);
    
    while (actualValue != expectedValue)
    {
        // ... but did not work out, as the expected value did not match the actual value.
        expectedValue = actualValue; 
    
    
        // Unpack the current average ...
        color = unpackUnorm4x8(actualValue);
        // ... and convert back to the sum.
        color.a *= MAX_COLOR_VALUES;
        color.rgb *= color.a;
                
        // Add the current color ...        
        color.rgb += addingColor.rgb;
        color.a += 1.0;
        
        // ... and normalize back.
        color.rgb /= color.a;
        color.a /= MAX_COLOR_VALUES;
     
     
        // Pack and ...
        newValue = packUnorm4x8(color);
        
        
        // ... try to store again ...
        actualValue = imageAtomicCompSwap(u_voxelGrid, pos, expectedValue, newValue);
    }
}

// see "Object-Order Ray Tracing for Fully Dynamic Scenes" in GPU Pro 5
void main(void)
{
    ivec3 gridSize = imageSize(u_voxelGrid);

    // Get xy-NDC of fragment.
    vec2 pos = (gl_FragCoord.xy / vec2(gridSize.xy)) * 2.0 - 1.0;

	// Discard pixels, which are not inside the AABB.
	if (pos.x < v_aabb.x || pos.y < v_aabb.y || pos.x > v_aabb.z || pos.y > v_aabb.w)
	{
		discard;
	}
	
	//
		
	ivec3 gridPosition;
	
    // Sample point location addresses the correct voxel grid entry ...
	gridPosition.xy = ivec2(gl_FragCoord.xy);
	
	// ... but has to be adjusted for z, as it is in the range of 0 and 1. 
    float depth = float(gridSize.z - 1);
	gridPosition.z = int(depth * gl_FragCoord.z);

	// Conservative depth rasterization preparation: Get half range, how much depth pixels are affected.    
    float halfDepthRange = abs(fwidth(gl_FragCoord.z) * 0.5);
	
	// Because of conservative depth rasterization, possibly a range has to be updated in the voxel grid. 
	ivec3 gridPositionStart = ivec3(gridPosition.xy, int(depth * clamp(gl_FragCoord.z - halfDepthRange, 0.0, 1.0)));
	ivec3 gridPositionEnd = ivec3(gridPosition.xy, int(depth * clamp(gl_FragCoord.z + halfDepthRange, 0.0, 1.0)));
	
	ivec3 gridPositionStep = ivec3(0, 0, 1);
	
	int gridRange = gridPositionEnd.z - gridPositionStart.z + 1;  
	
	ivec3 temp;
	
	// Swizzle back.
	if (v_orientation == 1.0)
    {
        temp.y = gridSize.y - gridPositionStart.z;
        temp.z = gridPositionStart.y;
    
        gridPositionStart.yz = temp.yz; 
        
        gridPositionStep = ivec3(0, -1, 0); 
    }
    else if (v_orientation == 2.0)
    {
        temp.x = gridSize.x - gridPositionStart.z;
        temp.z = gridPositionStart.x;
    
        gridPositionStart.xz = temp.xz; 
        
        gridPositionStep = ivec3(-1, 0, 0);
    }

    for (int i = 0; i < gridRange; i++)
    {
        // Note: Alpha channel contains the normalized counter.
	    imageAtomicAverage(gridPositionStart, vec4(texture(u_diffuse, v_texCoord).rgb, 1.0 / MAX_COLOR_VALUES));
	    
        gridPositionStart += gridPositionStep; 
    }	
}
