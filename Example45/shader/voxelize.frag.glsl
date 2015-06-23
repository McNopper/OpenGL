#version 440 core

// As 1 byte for the counter.
#define MAX_COLOR_VALUES 255.0

layout (binding = 1) uniform sampler2D u_diffuse;

uniform vec2 u_dimension;

in vec2 v_pos;
in vec4 v_aabb;

in vec2 v_texCoord;

in float v_orientation;

layout (r32ui, binding = 0) uniform coherent volatile uimage3D u_voxelGrid;

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
    while ((actualValue = imageAtomicCompSwap(u_voxelGrid, pos, expectedValue, newValue)) != expectedValue)
    {
        // ... but did not work out, as the expected value did not match the actual value.
        expectedValue = actualValue; 
    
    
        // Unpack the current average ...
        color = unpackUnorm4x8(actualValue);
        // ... and convert back to the sum.
        color.w *= MAX_COLOR_VALUES;
        color.rgb *= color.w;
                
        // Add the current color ...        
        color.rgb += addingColor.rgb;
        color.w += 1.0;
        
        // ... and normalize back.
        color.rgb /= color.w;
        color.w /= MAX_COLOR_VALUES;
     
     
        // Pack and try to store this value as the new value.
        newValue = packUnorm4x8(color);
    }
}

// see "Object-Order Ray Tracing for Fully Dynamic Scenes" in GPU Pro 5
void main(void)
{
	// Discard pixels, which are not inside the AABB.
	if (v_pos.x < v_aabb.x || v_pos.y < v_aabb.y || v_pos.x > v_aabb.z || v_pos.y > v_aabb.w)
	{
		discard;
	}

    ivec3 dim = imageSize(u_voxelGrid) - ivec3(1);
	
    // Conservative depth rasterization preparation: Get half range, how much depth pixels are affected.
    
    float halfDepthRange = fwidth(gl_FragCoord.z) * 0.5;
	
	//
		
	ivec3 gridPosition;
	
    // Sample point location addresses the correct voxel grid entry ...
	gridPosition.xy = ivec2(vec2(dim.xy) * (gl_FragCoord.xy / u_dimension));
	// ... but has to be adjusted for z, as it is in the range of 0 and 1. 
	gridPosition.z = int(float(dim.z) * gl_FragCoord.z);
	
	// Because of conservative depth rasterization, possibly a range has to be updated in the voxel grid. 
	ivec3 gridPositionStart = ivec3(gridPosition.xy, int(float(dim.z) * clamp(gl_FragCoord.z - halfDepthRange, 0.0, 1.0)));
	ivec3 gridPositionEnd = ivec3(gridPosition.xy, int(float(dim.z) * clamp(gl_FragCoord.z + halfDepthRange, 0.0, 1.0)));
	
	ivec3 gridPositionStep = ivec3(0, 0, 1);
	
	int gridRange = gridPositionEnd.z - gridPositionStart.z + 1;  
	
	// Swizzle back.
	if (v_orientation == 1.0)
    {
        gridPositionStart.yz = gridPositionStart.zy; 
        gridPositionEnd.yz = gridPositionEnd.zy;
        
        gridPositionStep = ivec3(0, 1, 0);
    }
    else if (v_orientation == 2.0)
    {
        gridPositionStart.xz = gridPositionStart.zx;
        gridPositionEnd.xz = gridPositionEnd.zx;
        
        gridPositionStep = ivec3(1, 0, 0);
    }

    for (int i = 0; i < gridRange; i++)
    {
        // Note: Alpha channel contains the normalized counter.
	    imageAtomicAverage(gridPositionStart, vec4(texture(u_diffuse, v_texCoord).rgb, 1.0 / MAX_COLOR_VALUES));
	    
        gridPositionStart += gridPositionStep; 
    }	
}
