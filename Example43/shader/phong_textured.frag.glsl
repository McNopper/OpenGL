#version 150

struct LightProperties
{
	vec3 direction;
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
};

struct MaterialProperties
{
	vec4 ambientColor;
	vec4 diffuseColor;
	vec4 specularColor;
	float specularExponent;
	sampler2D diffuseTexture;
};

uniform	LightProperties u_light;
uniform	MaterialProperties u_material;

uniform	int u_useTexture;

in vec3 v_normal;
in vec3 v_eye;
in vec2 v_texCoord;

out vec4 fragColor;

void main()
{
	// Note: All calculations are in camera space.


	vec4 textureColor = vec4(1.0, 1.0, 1.0, 1.0);
	if (u_useTexture != 0)
	{
		textureColor = texture(u_material.diffuseTexture, v_texCoord);
	}

	// We use the diffuse texture for the ambient factor as well
	vec4 color = u_light.ambientColor * u_material.ambientColor * textureColor;

	vec3 normal = normalize(v_normal);

	float nDotL = max(dot(u_light.direction, normal), 0.0);
		
	if (nDotL > 0.0)
	{
		vec3 eye = normalize(v_eye);
	
		// Incident vector is opposite light direction vector.
		vec3 reflection = reflect(-u_light.direction, normal);
		
		float eDotR = max(dot(eye, reflection), 0.0);
	
		color += u_light.diffuseColor * u_material.diffuseColor * textureColor * nDotL;
		
		float specularIntensity = 0.0;
		
		if (eDotR > 0.0)
		{
			specularIntensity = pow(eDotR, u_material.specularExponent);
		}
		
		color += u_light.specularColor * u_material.specularColor * specularIntensity;
	}

	fragColor = color;
}   