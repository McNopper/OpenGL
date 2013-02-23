#version 150

uniform mat4 u_modelViewProjectionMatrix;

in vec4 a_vertex;
in vec2 a_texCoord;

out vec2 v_texCoord;

void main(void)
{
	v_texCoord = a_texCoord;

	gl_Position = u_modelViewProjectionMatrix*a_vertex;
}
