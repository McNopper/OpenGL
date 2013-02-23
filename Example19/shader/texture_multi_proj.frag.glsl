#version 150

uniform sampler2D u_texture;
uniform sampler2D u_mirrorTexture; 

in float v_intensity;
in vec2 v_texCoord;
in vec4 v_projCoord;

out vec4 fragColor;

void main(void)
{
	// Compose the final image out of the ice picture and the dynamically created texture.
	fragColor = mix(texture(u_texture, v_texCoord) * v_intensity, textureProj(u_mirrorTexture, v_projCoord), 0.25);
}
