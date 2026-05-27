#version 460 core

uniform samplerCube u_texture;

in vec3 v_ray;

out vec4 fragColor;

void main(void)
{
    fragColor = texture(u_texture, normalize(v_ray));
}
