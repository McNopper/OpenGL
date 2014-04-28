#version 150

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;

uniform mat4 u_depthPassMatrix;

in vec4 a_vertex;
in vec3 a_normal;

out vec3 v_normal;
out vec4 v_projCoord;

void main(void)
{
	vec4 vertex = u_modelMatrix * a_vertex;

	// Bring the vertex to the light / depth pass coordinate system.
	v_projCoord = u_depthPassMatrix * vertex;

	vertex = u_viewMatrix * vertex;

	v_normal = u_normalMatrix * a_normal;

	gl_Position = u_projectionMatrix*vertex;
}
