#version 410 core

// Pre-filter the environment map for a given roughness level using GGX importance
// sampling with the N=V=R split-sum approximation (Karis / Epic 2013).
//
// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf

#define GLUS_PI     3.1415926535897932384626433832795
#define NUM_SAMPLES 512

uniform sampler2D u_panoramaTexture;
uniform int       u_face;
uniform float     u_roughness;

in  vec2 v_texCoord;
out vec4 fragColor;

// see http://gl.ict.usc.edu/Data/HighResProbes/
vec2 panorama(vec3 ray)
{
	return vec2(0.5 + 0.5*atan(ray.x, -ray.z)/GLUS_PI, 1.0 - acos(clamp(ray.y, -1.0, 1.0))/GLUS_PI);
}

// Same face mapping as panorama_to_cubemap.frag.glsl (OpenGL Table 8.19).
vec3 cubemapDirection(int face, vec2 uv)
{
	switch (face)
	{
		case 0: return normalize(vec3( 1.0, -uv.y, -uv.x)); // +X
		case 1: return normalize(vec3(-1.0, -uv.y,  uv.x)); // -X
		case 2: return normalize(vec3( uv.x,  1.0,  uv.y)); // +Y
		case 3: return normalize(vec3( uv.x, -1.0, -uv.y)); // -Y
		case 4: return normalize(vec3( uv.x, -uv.y,  1.0)); // +Z
		case 5: return normalize(vec3(-uv.x, -uv.y, -1.0)); // -Z
	}
	return vec3(0.0);
}

// Hammersley low-discrepancy sequence (Van der Corput radical inverse).
// see http://mathworld.wolfram.com/HammersleyPointSet.html
float radicalInverse_VdC(uint bits)
{
	bits = (bits << 16u) | (bits >> 16u);
	bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
	bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
	bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
	bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
	return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley(uint i, uint N)
{
	return vec2(float(i) / float(N), radicalInverse_VdC(i));
}

// GGX microfacet importance sampling — returns H in tangent space (z = up).
// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
vec3 importanceSampleGGX(vec2 xi, float roughness)
{
	float alpha     = roughness * roughness;
	float phi       = 2.0 * GLUS_PI * xi.x;
	float cosTheta  = sqrt((1.0 - xi.y) / (1.0 + (alpha*alpha - 1.0) * xi.y));
	float sinTheta  = sqrt(1.0 - cosTheta*cosTheta);
	return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

void main(void)
{
	// N = V = R (split-sum N=V approximation).
	vec3 N = cubemapDirection(u_face, v_texCoord);
	vec3 V = N;

	// Build an orthonormal basis around N.
	vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
	vec3 tangent   = normalize(cross(up, N));
	vec3 bitangent = cross(N, tangent);

	vec3  color       = vec3(0.0);
	float totalWeight = 0.0;

	for (uint i = 0u; i < uint(NUM_SAMPLES); i++)
	{
		vec2 xi    = hammersley(i, uint(NUM_SAMPLES));
		vec3 H_tan = importanceSampleGGX(xi, u_roughness);
		vec3 H     = normalize(tangent * H_tan.x + bitangent * H_tan.y + N * H_tan.z);

		vec3  L     = reflect(-V, H);
		float NdotL = max(dot(N, L), 0.0);

		if (NdotL > 0.0)
		{
			color       += texture(u_panoramaTexture, panorama(L)).rgb * NdotL;
			totalWeight += NdotL;
		}
	}

	fragColor = vec4(totalWeight > 0.0 ? color / totalWeight : color, 1.0);
}
