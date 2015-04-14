#version 150

layout(triangles) in;

// Conservative and "normal" rasterized triangle are created.
layout(triangle_strip, max_vertices = 6) out;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;

uniform vec2 u_halfPixelSize;

out vec2 v_pos;
out vec4 v_aabb;

out vec4 v_color;

// See Gpu Gems 2, Chapter 42: Conservative Rasterization. (http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter42.html)
// See "Conservative and Tiled Rasterization Using a Modified Triangle Setup." Journal of Graphics Tools. 
// See Plane-Plane Intersection (http://mathworld.wolfram.com/Plane-PlaneIntersection.html)
void main(void)
{
    int i;
    
    vec4 vertex[3];
    
    vec2 pos[3];

    // Axis aligned bounding box (AABB) initialized with maximum/minimum NDC values.
    vec4 aabb = vec4(1.0, 1.0, -1.0, -1.0);

    for (i = 0; i < gl_in.length(); i++)
    {
		vertex[i] = u_projectionMatrix * u_viewMatrix * gl_in[i].gl_Position;
		
		// Later NDC position if each fragment. 
		pos[i] = vertex[i].xy / vertex[i].w;  

		// Get AABB of the triangle.
		aabb.xy = min(aabb.xy, pos[i].xy);

		aabb.zw = max(aabb.zw, pos[i].xy);
    }
        
    //
    // Calculate triangle for conservative rasterization.
    //
    
    // Add offset of half pixel size to AABB.
    vec4 aabbConservative = aabb + vec4(-u_halfPixelSize, u_halfPixelSize);

	//
    
	vec3 plane[3];

    for (i = 0; i < gl_in.length(); i++)
    {
    	// Calculate the plane through each edge of the triangle.
        // The plane equation is A*x + B*y + C*w = 0. Note, that D component from the plane is zero, as it goes throught the origin. Also, w is used as the third dimension.
        //
    	// Note: The plane goes through the origin, as (vertex[i].x, vertex[i].y, vertex[i].w) are vectors from the origin.
    	// Note: A*x + B*y + C*w = 0 <=> A*x/w + B*y/w + C = 0 [w != 0] <=> A*xClip + B*yClip + C
    	//       A*xClip + B*yClip + C is a vector in 2D space. In this case, it is the normal of the edge in 2D space.
    	// 
    	// By calculating the cross product of the two vectors (the end points of the edge), we gather the normal of the plane.
		plane[i] = cross(vertex[i].xyw, vertex[(i + 2) % 3].xyw);
		
		// Move plane, by adjusting C.
		//
		// Note: A*(x+dx) + B*(y+dy) + C*w = 0 [Another plane with a specific distance d given by dx and dy] <=>
		//       A*x + A*dx + B*y + B*dy + C*w = 0 <=>
		//       A*xClip + B*yClip + C + A*dx + B*dy = 0
		//
		// Half pixel size is d. Absolute of plane's xy, as the sign is already in the normal of the plane. 
		plane[i].z -= dot(u_halfPixelSize, abs(plane[i].xy));
    }
    
    // Create conservative, red rasterized triangle ... 
    
    vec3 intersect;
        
    for (i = 0; i < gl_in.length(); i++)
    {
    	// As both planes go through the origin, the intersecting line goes also through the origin. This simplifies the intersection calculation.
    	// The intersecting line is perpendicular to both planes (see Wolfram MathWorld),
    	// so the intersection line is just the cross product of both normals. 
    	intersect = cross(plane[i], plane[(i+1) % 3]);
    
    	// The line is a direction (x, y, w) but projects to the same point in window space.
    	//
    	// Compare: (x, y, w) <=> (x/w, y/w, 1) => (xClip, yClip)
    	//
    	gl_Position.xyw = intersect;
		
		// Later NDC position if each fragment.
		v_pos = intersect.xy / intersect.z;
		v_aabb = aabbConservative;
		
		v_color = vec4(1.0, 0.0, 0.0, 1.0);

        EmitVertex();
    }
    
    EndPrimitive();
    
    // ... and create "normal", blue rasterized triangle for comparison.
    
    for (i = 0; i < gl_in.length(); i++)
    {
    	gl_Position = vertex[i];
		
		v_pos = pos[i];
		v_aabb = aabb;
		
		v_color = vec4(0.0, 0.0, 1.0, 1.0);

        EmitVertex();
    }
    
    EndPrimitive();            
}
