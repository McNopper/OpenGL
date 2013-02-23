#version 150

uniform float u_repeat;

in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
	float s = mod(floor(v_texCoord.s * u_repeat), 2.0);
	float t = mod(floor(v_texCoord.t * u_repeat), 2.0);

	if (s + t == 1.0)
	{
		discard;
	}

	fragColor = vec4(1.0, 1.0, 0.0, 0.5);
}
