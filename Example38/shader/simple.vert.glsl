#version 410 core

in vec4 a_vertex;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main(void)
{
	gl_Position = a_vertex;
}
