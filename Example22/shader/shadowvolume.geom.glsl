#version 150

layout(triangles_adjacency) in;

// Front cap 3 + back cap 3 + 3 * sides 4
layout(triangle_strip, max_vertices = 18) out;

uniform mat4 u_projectionMatrix;

uniform vec3 u_lightDirection;

void main(void)
{
	vec3 triangleVector[2];
	
	// Calculating the two vectors of the triangle to calculate the normal
	// see http://developer.download.nvidia.com/opengl/specs/GL_EXT_geometry_shader4.txt for which point has which index 
	for (int i = 0; i < 2; i++)
	{
		triangleVector[i] = (gl_in[i*2 + 2].gl_Position - gl_in[0].gl_Position).xyz;
	}

	vec3 triangleNormal = cross(triangleVector[0], triangleVector[1]);

	float dotNormal = dot(u_lightDirection, triangleNormal);

	// Triangle not facing the light
	if (dotNormal <= 0.0)
	{
		return;
	}
	
	// Front cap	
	gl_Position = u_projectionMatrix * gl_in[0].gl_Position;
    EmitVertex();
    
	gl_Position = u_projectionMatrix * gl_in[2].gl_Position;
    EmitVertex();

	gl_Position = u_projectionMatrix * gl_in[4].gl_Position;
	EmitVertex();

	EndPrimitive();
	
	// Back cap needs different winding order; extrude to infinity; assume we have a directional light	
	gl_Position = u_projectionMatrix * (- vec4(u_lightDirection, 0.0));
    EmitVertex();

	gl_Position = u_projectionMatrix * (-vec4(u_lightDirection, 0.0));
    EmitVertex();

	gl_Position = u_projectionMatrix * (-vec4(u_lightDirection, 0.0));
    EmitVertex();

	EndPrimitive();

    // Three sides of the front and back cap
	vec3 triangleAdjacentVector[2];
	vec3 triangleAdjacentNormal;
	float dotAdjacentNormal;
    	    	
    for (int i = 0; i < 3; i++)
   	{
		for (int k = 0; k < 2; k++)
		{
			triangleAdjacentVector[k] = (gl_in[(i*2 + k + 1) % 6].gl_Position - gl_in[i*2].gl_Position).xyz;
		}
	
		triangleAdjacentNormal = cross(triangleAdjacentVector[0], triangleAdjacentVector[1]);
	
		dotAdjacentNormal = dot(u_lightDirection, triangleAdjacentNormal);
		
		// Silhouette, so we need a side
		if (dotNormal * dotAdjacentNormal < 0.0)
		{
			gl_Position = u_projectionMatrix * gl_in[i*2].gl_Position;
	        EmitVertex();
		
			gl_Position = u_projectionMatrix * (-vec4(u_lightDirection, 0.0));
	        EmitVertex();

			gl_Position = u_projectionMatrix * gl_in[(i*2 + 2) % 6].gl_Position;
	        EmitVertex();
    
			gl_Position = u_projectionMatrix * (-vec4(u_lightDirection, 0.0));
	        EmitVertex();

	    	EndPrimitive();
		}
	}
}
