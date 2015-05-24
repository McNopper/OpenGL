#version 440 core

layout (location = 0) in vec4 a_vertex;

layout (location = 1) in vec2 a_texCoord;

out vec2 v_g_texCoord;

void main(void)
{
	v_g_texCoord = a_texCoord;

	gl_Position = a_vertex;
}
