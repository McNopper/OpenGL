#version 150

uniform mat3 u_normalMatrix;

in vec4 a_vertex;
in vec3 a_normal;

out vec3 g_normal;

void main(void)
{
	g_normal = u_normalMatrix * a_normal;

	// Projection etc. is done in the geometry shader
	gl_Position = a_vertex;
}
