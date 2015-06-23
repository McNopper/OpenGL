#version 440 core

layout(triangles) in;

layout(triangle_strip, max_vertices = 3) out;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;

uniform vec2 u_halfPixelSize;

in vec2 v_g_texCoord[];

out vec2 v_pos;
out vec4 v_aabb;

out vec2 v_texCoord;

out flat float v_orientation;

// See Gpu Gems 2, Chapter 42: Conservative Rasterization. (http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter42.html)
// See "Conservative and Tiled Rasterization Using a Modified Triangle Setup." Journal of Graphics Tools. 
// See Plane-Plane Intersection (http://mathworld.wolfram.com/Plane-PlaneIntersection.html)
void main(void)
{
    int i;
    
    vec4 vertex[3];
    
    for (i = 0; i < gl_in.length(); i++)
    {
        vertex[i] = u_projectionMatrix * u_viewMatrix * u_modelMatrix * gl_in[i].gl_Position;
        vertex[i] /= vertex[i].w; 
    }

    // Find largest face area and swizzle coordinates.

    vec3 triangleNormal = abs(normalize(cross(vertex[1].xyz - vertex[0].xyz, vertex[2].xyz - vertex[0].xyz)));    

    if (triangleNormal.x < triangleNormal.z && triangleNormal.y < triangleNormal.z)
    {
	    v_orientation = 0.0;
    }
    else if (triangleNormal.x < triangleNormal.y)
    {
        v_orientation = 1.0;
        
	    for (i = 0; i < gl_in.length(); i++)
	    {
	        vertex[i].yz = vertex[i].zy;  
	    }
    }
    else
    {
        v_orientation = 2.0;
        
        for (i = 0; i < gl_in.length(); i++)
        {
            vertex[i].xz = vertex[i].zx;  
        }
    }    

    //
    
    // Axis aligned bounding box (AABB) initialized with maximum/minimum NDC values.
    vec4 aabb = vec4(1.0, 1.0, -1.0, -1.0);

    for (i = 0; i < gl_in.length(); i++)
    {
		// Get AABB of the triangle.
		aabb.xy = min(aabb.xy, vertex[i].xy);

		aabb.zw = max(aabb.zw, vertex[i].xy);
    }
        
    //
    // Calculate triangle for conservative rasterization.
    //
    
    // Add offset of half pixel size to AABB.
    aabb += vec4(-u_halfPixelSize, u_halfPixelSize);

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
    
    //
    // Triangle plane to later calculate the new z coordinate.
    //

    vec4 trianglePlane;
        
    trianglePlane.xyz = normalize(cross(vertex[1].xyz - vertex[0].xyz, vertex[2].xyz - vertex[0].xyz));
        
    trianglePlane.w = -dot(vertex[0].xyz, trianglePlane.xyz);
    
    //    
    // Create conservative rasterized triangle.
    // 
    
    vec3 intersect;
        
    for (i = 0; i < gl_in.length(); i++)
    {
    	// As both planes go through the origin, the intersecting line goes also through the origin. This simplifies the intersection calculation.
    	// The intersecting line is perpendicular to both planes (see Wolfram MathWorld),
    	// so the intersection line is just the cross product of both normals. 
    	intersect = cross(plane[i], plane[(i+1) % 3]);
    	
        //
        // The line is a direction (x, y, w) but projects to the same point in window space.
        //
        // Compare: (x, y, w) <=> (x/w, y/w, 1) => (xClip, yClip)
        //
    	intersect /= intersect.z; 
    
        gl_Position.xy = intersect.xy;
        
        // Calculate the new z-Coordinate derived from a point on a plane.
        
        gl_Position.z = -(trianglePlane.x * intersect.x + trianglePlane.y * intersect.y + trianglePlane.w) / trianglePlane.z;   
          
        gl_Position.w = 1.0;
		
		// Later NDC position if each fragment.
		v_pos = intersect.xy;
		v_aabb = aabb;
		
		v_texCoord = v_g_texCoord[i];

        EmitVertex();
    }
    
    EndPrimitive();
}
