#version 410 core

in vec3 v_normal;

out vec4 fragColor;

void main(void)
{
	// Normal is in world space.
	vec3 normal = normalize(v_normal);

	// TODO: Do BRDF and IBL based rendering.
	
	fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}
