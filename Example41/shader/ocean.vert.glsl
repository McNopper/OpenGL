#version 430 core

uniform mat4 u_modelViewProjectionMatrix;
uniform mat3 u_normalMatrix;

in vec4 a_vertex;
in vec4 a_normal;

out vec3 v_normal;

void main(void)
{
	v_normal = u_normalMatrix * a_normal.xyz;

	gl_Position = u_modelViewProjectionMatrix * a_vertex;
}
