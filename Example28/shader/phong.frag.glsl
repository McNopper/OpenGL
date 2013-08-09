#version 410 core

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
};

uniform	LightProperties u_light;
uniform	MaterialProperties u_material;

in vec3 v_normal;
in vec3 v_eye;

layout(location = 0) out vec4 fragColor;
layout(location = 1) out vec4 normalColor;

void main()
{
	// Note: All calculations are in camera space.

	vec4 color = u_light.ambientColor * u_material.ambientColor;

	vec3 normal = normalize(v_normal);

	float nDotL = max(dot(u_light.direction, normal), 0.0);
		
	if (nDotL > 0.0)
	{
		vec3 eye = normalize(v_eye);
	
		// Incident vector is opposite light direction vector.
		vec3 reflection = reflect(-u_light.direction, normal);
		
		float eDotR = max(dot(eye, reflection), 0.0);
	
		color += u_light.diffuseColor * u_material.diffuseColor * nDotL;
		
		float specularIntensity = 0.0;
		
		if (eDotR > 0.0)
		{
			specularIntensity = pow(eDotR, u_material.specularExponent);
		}
		
		color += u_light.specularColor * u_material.specularColor * specularIntensity;
	}

	fragColor = color;
	
	//
	
	normalColor = vec4(normal.x * 0.5 + 0.5, normal.y * 0.5 + 0.5, normal.z * 0.5 + 0.5, 1.0);
}   