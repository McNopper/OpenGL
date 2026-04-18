#version 410 core

uniform samplerCube u_texture;

in vec3 v_ray;

out vec4 fragColor;

void main(void)
{
	vec3 ray = normalize(v_ray);

	fragColor = texture(u_texture, ray);
}
