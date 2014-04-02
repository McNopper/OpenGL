#version 410 core

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;

uniform vec4 u_eye;

in vec4 a_vertex;
in vec3 a_normal;

out vec3 v_eye;
out vec3 v_normal;

void main(void)
{
	v_normal = u_normalMatrix * a_normal;
	
	vec4 vertex = u_modelMatrix * a_vertex;
	
	v_eye = (u_eye - vertex).xyz; 

	gl_Position = u_viewProjectionMatrix * vertex;
}
