#version 150

// The higher the value, the bigger the contrast between the fur length.
#define FUR_STRENGTH_CONTRAST 2.0

// The higher the value, the less fur.
#define FUR_STRENGTH_CAP 0.3

uniform sampler2D u_textureFurColor;

uniform sampler2D u_textureFurStrength;

uniform vec3 u_lightDirection;

in vec3 v_normal;
in vec2 v_texCoord;

in float v_furStrength;

out vec4 fragColor;

void main()
{
	// Note: All calculations are in camera space.

	vec3 normal = normalize(v_normal);

	// Orthogonal fur to light is still illumintated. So shift by one, that only fur targeting away from the light do get darkened. 
	float intensity = clamp(dot(normal, u_lightDirection) + 1.0, 0.0, 1.0);
	
	float furStrength = clamp(v_furStrength * texture(u_textureFurStrength, v_texCoord).r * FUR_STRENGTH_CONTRAST - FUR_STRENGTH_CAP, 0.0, 1.0);

	fragColor = vec4(texture(u_textureFurColor, v_texCoord).rgb * intensity, furStrength);
}   