#version 150

struct LightProperties
{
	vec3 direction;
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
};

uniform	LightProperties u_light;

uniform sampler2D u_textureFurColor;

in vec3 v_normal;
in vec2 v_texCoord;
in vec3 v_eye;

out vec4 fragColor;

void main()
{
	// Note: All calculations are in camera space.
	
	vec4 textureColor = texture(u_textureFurColor, v_texCoord);

	vec4 color = u_light.ambientColor * textureColor;

	vec3 normal = normalize(v_normal);

	float nDotL = max(dot(u_light.direction, normal), 0.0);
		
	if (nDotL > 0.0)
	{
		vec3 eye = normalize(v_eye);
	
		// Incident vector is opposite light direction vector.
		vec3 reflection = reflect(-u_light.direction, normal);
		
		float eDotR = max(dot(eye, reflection), 0.0);
	
		color += u_light.diffuseColor * textureColor * nDotL;
	}

	fragColor = color;
}   