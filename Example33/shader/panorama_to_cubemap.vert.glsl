#version 410 core

layout(location = 0) in vec2 a_vertex;

out vec2 v_texCoord;

void main(void)
{
	// Pass NDC position directly as texcoord; fragment shader uses it as the face uv in [-1, 1].
	v_texCoord = a_vertex;

	gl_Position = vec4(a_vertex, 0.0, 1.0);
}
