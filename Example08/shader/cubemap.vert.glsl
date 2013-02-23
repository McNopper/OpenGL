#version 150

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalMatrix;
uniform mat3 u_inverseViewMatrix;

in vec4 a_vertex;
in vec3 a_normal;

out vec3 v_reflect;

void main(void)
{
	// Calculate first in camera / view space.

	vec4 vertex = u_modelViewMatrix * a_vertex;
	
	vec3 normal = u_normalMatrix * a_normal;
	
	vec3 incident = vec3(vertex); 

	vec3 reflectView = reflect(incident, normal);
	
	// Now go back from view space, as the cube map is in world space.
	
	v_reflect = u_inverseViewMatrix * reflectView;

	gl_Position = u_projectionMatrix * vertex;
}
