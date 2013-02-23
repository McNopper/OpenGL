#version 150

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;

in vec4 a_vertex;

void main(void)
{
	gl_Position = u_projectionMatrix*u_modelViewMatrix*a_vertex;
}
