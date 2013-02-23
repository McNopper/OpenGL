#version 150

uniform sampler2D u_texture; 

in float v_intensity;
in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
	fragColor = texture(u_texture, v_texCoord) * v_intensity;
}
