#version 150

uniform sampler2D u_texture; 

in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
	vec4 color = texture(u_texture, v_texCoord);

	if (v_texCoord.s >= 0.5)
	{
		float grey = color.r*0.299 + color.g*0.587 + color.b*0.114;
		
		fragColor = vec4(grey, grey, grey, 1.0f);
	}
	else
	{
		fragColor = color;
	}
}
