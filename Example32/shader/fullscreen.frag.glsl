#version 410 core

uniform sampler2D u_fullscreenTexture; 

uniform float u_exposure;
uniform float u_gamma;

in vec2 v_texCoord;

out vec4 fragColor;

void main(void)
{
	// TODO As soon as MSAA texture is used, do resolve.
	vec3 hdrColor = texture(u_fullscreenTexture, v_texCoord).rgb;

	vec3 toneMappedColor = 1.0 - exp2(-hdrColor * u_exposure); 
	
	vec3 gammaCorrectedColor = pow(toneMappedColor, vec3(1.0 / u_gamma));

	fragColor = vec4(gammaCorrectedColor, 1.0);		
}
