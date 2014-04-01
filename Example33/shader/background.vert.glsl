#version 410 core

uniform mat4 u_viewProjectionMatrix;

in vec4 a_vertex;

out vec3 v_ray;

void main(void)
{
	// As the rendered object is a sphere, we can use its vertex position as the ray to look up in the texture.
	v_ray = normalize(a_vertex.xyz);
	
	gl_Position = u_viewProjectionMatrix * a_vertex;
}
