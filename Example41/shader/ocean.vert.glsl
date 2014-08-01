#version 430 core

layout(binding = 0, rg32f) uniform sampler2D u_displacementMap;

uniform mat4 u_modelViewProjectionMatrix;
uniform mat3 u_normalMatrix;

in vec4 a_vertex;
in vec2 a_texCoord;

out vec3 v_normal;
out vec2 v_texCoord;

void main(void)
{
	// TODO Calculate normal.
	v_normal = u_normalMatrix * vec3(0.0, 1.0, 0.0);
	
	v_texCoord = a_texCoord;

	// TODO Do displacement.
	gl_Position = u_modelViewProjectionMatrix * a_vertex;
}
