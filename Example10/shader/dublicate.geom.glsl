#version 150

// A triangle with three points come in
layout(triangles) in;

// Out of three triangle points, we create a triangle strip with two triangles
layout(triangle_strip, max_vertices = 6) out;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform vec4 u_lightPosition;

in vec3 g_normal[3];

out vec3 v_normal;
out vec3 v_eye;
out vec4 v_color;
out vec3 v_lightDirection;

void main(void)
{
    int i;

	mat4 translateMatrix = mat4(1.0);
	
	vec4 position;

	translateMatrix[3][0] = -1.5;
	
    for (i=0; i < gl_in.length(); i++)
    {
        position = u_viewMatrix*translateMatrix*gl_in[i].gl_Position;

		gl_Position = u_projectionMatrix*position;

		// Normal is already in camera space
		v_normal = g_normal[i];
		
		v_eye = -vec3(position);
		
		v_color = vec4(1.0, 0.0, 0.0, 1.0);

		// Light position is in camera space
		v_lightDirection = vec3(u_lightPosition - position);

        EmitVertex();
    }
    
    EndPrimitive();    

	translateMatrix[3][0] = +1.5;
	
    for (i=0; i < gl_in.length(); i++)
    {
        position = u_viewMatrix*translateMatrix*gl_in[i].gl_Position;

		gl_Position = u_projectionMatrix*position;

		v_normal = g_normal[i];
		
		v_eye = -vec3(position);
		
		v_color = vec4(0.0, 0.0, 1.0, 1.0);

		v_lightDirection = vec3(u_lightPosition - position);

        EmitVertex();
    }
    
    EndPrimitive();    
}
