#version 150

uniform sampler2DArray u_framebufferTexture;

uniform int u_layers; 

in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
	int i;
	
	float layer = float(u_layers - 1);
	
	vec4 dstColor = vec4(texture(u_framebufferTexture, vec3(v_texCoord, layer)).rgb, 1.0);
	
	vec4 srcColor;
	
	// Back to front blending.
	for (i = u_layers - 2; i >= 0; i--)
	{	
		layer = float(i);
	
		srcColor = texture(u_framebufferTexture, vec3(v_texCoord, layer));
		
		// Blend using "default" blend equation.
		dstColor = vec4(mix(dstColor.rgb, srcColor.rgb, srcColor.a), 1.0);
	}

	fragColor = dstColor;		
}
