#version 410 core

uniform samplerCubeArray u_texture;

in vec3 v_ray;

out vec4 fragColor;

void main(void)
{
	vec3 ray = normalize(v_ray);

	// Roughness 0.0 can be used as the environment.
	fragColor = texture(u_texture, vec4(ray, 0.0));
}
