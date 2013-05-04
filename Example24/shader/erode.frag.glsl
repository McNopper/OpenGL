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
};

uniform	LightProperties u_light;
uniform	MaterialProperties u_material;

uniform sampler3D u_texture;

uniform float u_erode;

in vec3 v_normal;
in vec3 v_eye;
in vec3 v_vertex;

out vec4 fragColor;

void main()
{
	float erodeLevel = texture(u_texture, normalize(v_vertex) * 0.5 + 0.5).r;
	
	// u_erode gets smaller over time. If this value is smaller than the reference texel, discard it.
	// This results in an erode effect over time.
	if (erodeLevel > u_erode)
	{
		discard;
	}

	// If not dicarded, render standard phong.

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
}   