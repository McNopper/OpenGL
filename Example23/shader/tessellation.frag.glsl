#version 410 core

uniform mat3 u_normalMatrix;

uniform sampler2D u_colorTexture;
uniform sampler2D u_normalTexture;

in vec3 v_tangent;
in vec3 v_bitangent;
in vec3 v_normal;
in vec2 v_texCoord;

layout(location = 0, index = 0) out vec4 fragColor;

void main(void)
{
	mat3 toObjectSpace = mat3(normalize(v_tangent), normalize(v_bitangent), normalize(v_normal));

	vec3 normal = normalize(texture(u_normalTexture, v_texCoord).xyz * 2.0 - 1.0);
	// No conversion needed. Seems that this normal map uses the OpenGL axis
	normal = u_normalMatrix * toObjectSpace * normal; 

	vec3 lightDirection = normalize(vec3(0.0, 1.0, 1.0));

	float brightness = clamp(dot(lightDirection, normal), 0.25, 1.0); 	

	fragColor = texture(u_colorTexture, v_texCoord) * brightness;
}
