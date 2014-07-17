#version 430 core

uniform vec3 u_lightDirection;
uniform vec4 u_color;

in vec3 v_normal;

out vec4 fragColor;

void main(void)
{
	fragColor = u_color * max(dot(normalize(v_normal), u_lightDirection), 0.0);
}
