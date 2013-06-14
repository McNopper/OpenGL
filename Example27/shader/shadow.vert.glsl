#version 150

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_shadowProjectionMatrix;
uniform mat4 u_modelMatrix;

in vec4 a_vertex;

void main(void)
{
	gl_Position = u_projectionMatrix * u_viewMatrix * u_shadowProjectionMatrix * u_modelMatrix * a_vertex;
}
