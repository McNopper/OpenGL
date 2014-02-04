#version 410 core

#define GLUS_PI 3.1415926535897932384626433832795

uniform sampler2D u_panoramaTexture;
uniform vec3 u_colorMaterial;

uniform uint u_numberSamples;
uniform uint u_m;
uniform float u_binaryFractionFactor;

in vec3 v_tangent;
in vec3 v_bitangent;
in vec3 v_normal;

out vec4 fragColor;

//
// Map from normalized ray to st-coordinate of the panorma texture.
//
// see http://gl.ict.usc.edu/Data/HighResProbes/ 
// Note: that conversion has to be adapted to st-coordinates and the range of s to [0..1] 
//
vec2 panorama(vec3 ray)
{
	// Note: Two arguments of atan is atan2
	return vec2(0.5 + 0.5*atan(ray.x, -ray.z)/GLUS_PI, 1.0 - acos(ray.y)/GLUS_PI);
}

// see http://mathworld.wolfram.com/HammersleyPointSet.html
// see https://github.com/wdas/brdf/blob/master/src/shaderTemplates/brdfIBL.frag
vec2 hammersley(uint originalSample)
{
	uint revertSample;

	// Revert bits by swapping blockwise. Lower bits are moved up and higher bits down.
	revertSample = (originalSample << 16u) | (originalSample >> 16u);
	revertSample = ((revertSample & 0x00ff00ffu) << 8u) | ((revertSample & 0xff00ff00u) >> 8u);
	revertSample = ((revertSample & 0x0f0f0f0fu) << 4u) | ((revertSample & 0xf0f0f0f0u) >> 4u);
	revertSample = ((revertSample & 0x33333333u) << 2u) | ((revertSample & 0xccccccccu) >> 2u);
	revertSample = ((revertSample & 0x55555555u) << 1u) | ((revertSample & 0xaaaaaaaau) >> 1u);

	// Shift back, as only m bits are used.
	revertSample = revertSample >> (32 - u_m);

	return vec2(float(revertSample) * u_binaryFractionFactor, float(originalSample) * u_binaryFractionFactor);
}

// see Physically Based Rendering Chapter 13.6.1 and 13.6.3
vec3 cosineWeightedSampling(vec2 e)
{
	float x = sqrt(1.0 - e.x) * cos(2.0*GLUS_PI*e.y);
	float y = sqrt(1.0 - e.x) * sin(2.0*GLUS_PI*e.y);
	float z = sqrt(e.x);
	
	return vec3(x, y, z);
}

// see http://www.scratchapixel.com/lessons/3d-advanced-lessons/things-to-know-about-the-cg-lighting-pipeline/what-is-a-brdf/
// see Physically Based Rendering Chapter 5.6.1, 13.2 and 13.6.3
// see Fundamentals of Computer Graphics Chapter 14.2 and 24.2
vec3 brdfLambert(vec2 randomPoint, mat3 basis)
{
	vec3 rayTangentSpace = cosineWeightedSampling(randomPoint);
	
	// Transform ray to world space.
	vec3 ray = basis * rayTangentSpace;  

	//
	// Lo = BRDF*Li*cos()/PDF
	//
	// BRDF is color/PI and PDF is cos()/PI. cos() and PI are canceled out, which results in this formula: Lo = color * Li
	return u_colorMaterial * texture(u_panoramaTexture, panorama(ray)).rgb;
}

void main(void)
{
	vec3 color = vec3(0.0, 0.0, 0.0);
	
	// Tangent, Bitangent and Normal are in world space.
	vec3 tangent = normalize(v_tangent);
	vec3 bitangent = normalize(v_bitangent);
	vec3 normal = normalize(v_normal);

	mat3 basis = mat3(tangent, bitangent, normal);
	
	vec2 randomPoint;

	for (uint sampleIndex = 0; sampleIndex < u_numberSamples; sampleIndex++)
	{
		randomPoint = hammersley(sampleIndex);
	
		color += brdfLambert(randomPoint, basis);
		
		// TODO BRDF specular.
	}
	
	fragColor = vec4(color / float(u_numberSamples), 1.0);
}
