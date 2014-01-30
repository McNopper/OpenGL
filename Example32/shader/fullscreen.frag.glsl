#version 410 core

uniform sampler2DMS u_fullscreenTexture; 

uniform int u_msaaSamples;

uniform float u_exposure;
uniform float u_gamma;

in vec2 v_texCoord;

out vec4 fragColor;

vec4 sampleMS(sampler2DMS texture, ivec2 ipos)
{
	vec4 texel = vec4(0.0);	
    
    for (int i = 0; i < u_msaaSamples; i++)
    {
    	texel += texelFetch(texture, ipos, i);
    }
    
    texel /= float(u_msaaSamples);
    
    return texel;
}

void main(void)
{
	// MSAA sampling.

	vec2 fullscreenTextureSize = vec2(textureSize(u_fullscreenTexture));

	ivec2 itexCoord = ivec2(fullscreenTextureSize * v_texCoord);

	vec3 hdrColor = sampleMS(u_fullscreenTexture, itexCoord).rgb;
	
	// Tone mapping and gamma correction.

	vec3 toneMappedColor = 1.0 - exp2(-hdrColor * u_exposure); 
	
	vec3 gammaCorrectedColor = pow(toneMappedColor, vec3(1.0 / u_gamma));

	fragColor = vec4(gammaCorrectedColor, 1.0);		
}
