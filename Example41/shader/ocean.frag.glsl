#version 430 core

// TODO Remove texture, just for debugging purpose right now.
layout(binding = 0) uniform sampler2D u_displacementMap;

uniform vec3 u_lightDirection;
uniform vec4 u_color;

in vec3 v_normal;
in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
	float height = texture(u_displacementMap, v_texCoord).r;

	// TODO Remove texture, just for debugging purpose right now.
	fragColor = vec4(height, height, height, 1.0);
	//fragColor = u_color * max(dot(normalize(v_normal), u_lightDirection), 0.0);
}
