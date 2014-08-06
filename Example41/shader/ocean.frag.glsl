#version 430 core

layout(binding = 1) uniform sampler2D u_normalMap;

uniform mat3 u_normalMatrix;
uniform vec3 u_lightDirection;
uniform vec4 u_color;

in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
	vec3 normal = u_normalMatrix * normalize(texture(u_normalMap, v_texCoord).xyz);

	fragColor = u_color * max(dot(normal, u_lightDirection), 0.0);
}
