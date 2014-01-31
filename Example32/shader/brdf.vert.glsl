#version 410 core

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;

in vec4 a_vertex;
in vec3 a_normal;

out vec3 v_normal;

void main(void)
{
	v_normal = u_normalMatrix * v_normal;

	gl_Position = u_viewProjectionMatrix * u_modelMatrix * a_vertex;
}
