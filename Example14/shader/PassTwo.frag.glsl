#version 410 core

layout(location = 0, index = 0) out vec4 fragColor;

uniform sampler2D u_colorMapTexture;

in vec2 v_texCoord;

in float v_intensity;

void main(void)
{
	fragColor = texture(u_colorMapTexture, v_texCoord) * v_intensity;
}
