#version 460 core

uniform sampler2DMS u_framebufferTexture;
uniform int         u_msaaSamples;
uniform float       u_gamma;

in  vec2 v_texCoord;
out vec4 fragColor;

void main(void)
{
	ivec2 texelCoord = ivec2(v_texCoord * textureSize(u_framebufferTexture));

	vec4 hdrColor = vec4(0.0);
	for (int i = 0; i < u_msaaSamples; ++i)
		hdrColor += texelFetch(u_framebufferTexture, texelCoord, i);
	hdrColor /= float(u_msaaSamples);

	// Exposure tone-mapping then gamma correction
	vec3 mapped = 1.0 - exp2(-hdrColor.rgb);
	mapped = pow(mapped, vec3(1.0 / u_gamma));

	fragColor = vec4(mapped, 1.0);
}
