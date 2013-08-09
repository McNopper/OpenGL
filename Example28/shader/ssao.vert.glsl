#version 410 core

in vec4 a_vertex;
in vec2 a_texCoord;

out vec2 v_texCoord;

void main(void)
{
	v_texCoord = a_texCoord;

	gl_Position = a_vertex;
}
