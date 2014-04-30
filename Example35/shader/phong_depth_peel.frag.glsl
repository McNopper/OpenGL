#version 150

#define PEEL_BIAS 0.001 

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
	float alpha;
};

uniform	LightProperties u_light;
uniform	MaterialProperties u_material;

uniform sampler2DShadow u_peelTexture;

uniform int u_layer;

in vec4 v_projCoord;
in vec3 v_normal;
in vec3 v_eye;

out vec4 fragColor;

void main()
{
	// Peeling depth pass. Everything "before" is discarded/peeled, so behind fragments are used.
	// see http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.18.9286&rep=rep1&type=pdf  
	if (u_layer > 0 && textureProj(u_peelTexture, v_projCoord) < 1.0)
	{
		discard;
	}

	// Note: All calculations are in camera space.

	vec4 color = u_light.ambientColor * u_material.ambientColor;

	vec3 normal = normalize(v_normal);

	float nDotL = dot(u_light.direction, normal);
	
	// Back faces are rendered as well, so revert.
	if (nDotL < 0.0)
	{		
		normal = -normal;
		
		nDotL = -nDotL;
	}
		
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

	// Using a global alpha for the material.
	fragColor = vec4(color.rgb, u_material.alpha);
	
	// Writing own depth value, as we need the exact value for later comparison.
	// Please note, that on AMD hardware, PEEL_BIAS can be 0.0. This is a workaround for NVIDIA hardware.
	// see http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.18.9286&rep=rep1&type=pdf "An Invariance Issue"
	gl_FragDepth = v_projCoord.z / v_projCoord.w + PEEL_BIAS;
}   