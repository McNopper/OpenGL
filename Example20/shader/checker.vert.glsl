#version 150

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat2 u_textureMatrix;

in vec4 a_vertex;
in vec2 a_texCoord;

out vec2 v_texCoord;

void main(void)
{
	v_texCoord = u_textureMatrix * a_texCoord;

	gl_Position = u_projectionMatrix * u_modelViewMatrix * a_vertex;
}
