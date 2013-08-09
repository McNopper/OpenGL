#version 410 core

uniform sampler2D u_colorTexture;

uniform sampler2D u_ssaoTexture;

uniform vec2 u_texelStep;

uniform float u_noSSAO;

in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
	float result = 0.0f;

	for (int t = -2; t <= 2; t++)
	{
		for (int s = -2; s <= 2; s++)
		{
			vec2 offset = vec2(v_texCoord.s + float(s) * u_texelStep.s, v_texCoord.t + float(t) * u_texelStep.t);
			
			// Grey scale, so only one component is needed.
			result += texture(u_ssaoTexture, offset).r;
		}
	}	
	result /= 25.0;
	
	result = max(u_noSSAO, result);
	
	// For debug purpose: Render out the occlusion factor.
	//fragColor = vec4(result, result, result, 1.0);

	fragColor = texture(u_colorTexture, v_texCoord) * result;
}
