#version 150

uniform sampler2D u_texture; 

in float v_alpha;

out vec4 fragColor;

void main(void)
{
	// Red fading out sprite
	fragColor = texture(u_texture, gl_PointCoord)*vec4(1.0, 0.0, 0.0, v_alpha);
}
