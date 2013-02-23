#version 430 core

uniform sampler2D u_texture; 

in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
	fragColor = texture(u_texture, v_texCoord);
}
