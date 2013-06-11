#version 410 core

layout(location = 0) in vec2 a_vertex;

void main(void)
{
	gl_Position = vec4(a_vertex.x, 0.0, a_vertex.y, 1.0);
}
