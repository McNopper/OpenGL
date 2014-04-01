#version 430 core

#define GLUS_PI 3.1415926535897932384626433832795

uniform samplerCubeArray u_texture;

in vec3 v_ray;

out vec4 fragColor;

void main(void)
{
	vec3 ray = normalize(v_ray);

	fragColor = texture(u_texture, vec4(ray, 0.0));
}
