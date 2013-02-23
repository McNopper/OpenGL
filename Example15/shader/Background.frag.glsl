#version 150

uniform samplerCube u_cubemap;

in vec3 v_ray;

out vec4 fragColor;

void main(void)
{
	fragColor = texture(u_cubemap, normalize(v_ray));
}
