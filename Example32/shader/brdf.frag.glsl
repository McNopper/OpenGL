#version 410 core

#define GLUS_PI 3.1415926535897932384626433832795

uniform sampler2D u_panoramaTexture;
uniform vec3 u_colorMaterial;
uniform float u_roughnessMaterial;
uniform float u_R0Material;

uniform uint u_numberSamples;
uniform uint u_m;
uniform float u_binaryFractionFactor;

in vec3 v_eye;

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
	vec3 LtangentSpace = cosineWeightedSampling(randomPoint);
	
	// Transform light ray to world space.
	vec3 L = basis * LtangentSpace;  

	//
	// Lo = BRDF * L * NdotL / PDF
	//
	// BRDF is color / PI.
	// PDF is NdotL  /PI.
	// NdotL and PI are canceled out, which results in this formula: Lo = color * L
	return u_colorMaterial * texture(u_panoramaTexture, panorama(L)).rgb;
}

// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// see http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v2.pdf
// see Physically Based Rendering Chapter 13.6.1
vec3 microfacetWeightedSampling(vec2 e)
{
	float alpha = u_roughnessMaterial * u_roughnessMaterial;
	
	// Note: Polar Coordinates
	// x = sin(theta)*cos(phi)
	// y = sin(theta)*sin(phi)
	// z = cos(theta)
	
	float phi = 2.0 * GLUS_PI * e.y; 	
	float cosTheta = sqrt((1.0 - e.x) / (1.0 + (alpha*alpha - 1.0) * e.x));
	float sinTheta = sqrt(1.0 - cosTheta*cosTheta); 

	float x = sinTheta * cos(phi);
	float y = sinTheta * sin(phi);
	float z = cosTheta;

	return vec3(x, y, z);
}

// see http://en.wikipedia.org/wiki/Schlick%27s_approximation
float fresnelSchlick(float VdotH)
{
	return u_R0Material + (1.0 - u_R0Material) * pow(1.0 - VdotH, 5.0);
}

// see http://graphicrants.blogspot.de/2013/08/specular-brdf-reference.html
// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float geometricShadowingSchlickBeckmann(float NdotV, float k)
{
	return NdotV / (NdotV * (1.0 - k) + k);
}

// see http://graphicrants.blogspot.de/2013/08/specular-brdf-reference.html
// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
float geometricShadowingSmith(float NdotL, float NdotV, float k)
{
	return geometricShadowingSchlickBeckmann(NdotL, k) * geometricShadowingSchlickBeckmann(NdotV, k);
}

// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// see http://blog.selfshadow.com/publications/s2012-shading-course/burley/s2012_pbs_disney_brdf_notes_v2.pdf 
// see http://sirkan.iit.bme.hu/~szirmay/scook.pdf
vec3 brdfCookTorrance(vec2 randomPoint, mat3 basis, vec3 N, vec3 V, float k)
{
	vec3 noColor = vec3(0.0, 0.0, 0.0);

	vec3 HtangentSpace = microfacetWeightedSampling(randomPoint);
	
	// Transform H to world space.
	vec3 H = basis * HtangentSpace;
	
	// Note: reflect takes incident vector.
	vec3 L = reflect(-V, H);
	
	float NdotL = dot(N, L);
	float NdotV = dot(N, V);
	float NdotH = dot(N, H);
	
	// Lighted and visible
	if (NdotL > 0.0 && NdotV > 0.0)
	{
		float VdotH = dot(V, H);

		// Fresnel		
		float F = fresnelSchlick(VdotH);
		
		// Geometric Shadowing
		float G = geometricShadowingSmith(NdotL, NdotV, k);
	
		//
		// Lo = BRDF * L * NdotL / PDF
		//
		// BRDF is D * F * G / (4 * NdotL * NdotV).
		// PDF is D * NdotH / (4 * VdotH).
		// D and 4 * NdotL are canceled out, which results in this formula: Lo = color * L * F * G * VdotH / (NdotV * NdotH)		
		float colorFactor = F * G * VdotH / (NdotV * NdotH);
		
		// Note: Needed for robustness. With specific parameters, a NaN can be the result.
		if (isnan(colorFactor))
		{
			return noColor;
		}
				
		return texture(u_panoramaTexture, panorama(L)).rgb * colorFactor;
	}
	
	return noColor;
}

void main(void)
{
	vec3 color = vec3(0.0, 0.0, 0.0);
	
	vec3 eye = normalize(v_eye);
	
	// Tangent, Bitangent and Normal are in world space.
	vec3 tangent = normalize(v_tangent);
	vec3 bitangent = normalize(v_bitangent);
	vec3 normal = normalize(v_normal);

	mat3 basis = mat3(tangent, bitangent, normal);
	
	vec2 randomPoint;
	
	// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf Section Specular G
	float k = (u_roughnessMaterial + 1.0) * (u_roughnessMaterial + 1.0) / 8.0;

	for (uint sampleIndex = 0; sampleIndex < u_numberSamples; sampleIndex++)
	{
		randomPoint = hammersley(sampleIndex);
	
		// Diffuse
		color += brdfLambert(randomPoint, basis);
		
		// Specular
		color += brdfCookTorrance(randomPoint, basis, normal, eye, k);
	}
	
	fragColor = vec4(color / float(u_numberSamples), 1.0);
}
