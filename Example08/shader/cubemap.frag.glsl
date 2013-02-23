#version 150

uniform samplerCube u_cubemapTexture;

in vec3 v_reflect;

out vec4 fragColor;

void main(void)
{
	fragColor = texture(u_cubemapTexture, v_reflect);
}
