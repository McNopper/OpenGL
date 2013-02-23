#version 150

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;

in vec4 a_vertex;
in vec3 a_normal;

out vec3 v_ray;

void main(void)
{
	v_ray = normalize(a_vertex.xyz);
	
	gl_Position = u_projectionMatrix*u_modelViewMatrix*a_vertex;
}
