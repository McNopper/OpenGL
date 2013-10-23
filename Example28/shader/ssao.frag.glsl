#version 410 core

#define KERNEL_SIZE 32

// This constant removes artifacts caused by neighbour fragments with minimal depth difference.
#define CAP_MIN_DISTANCE 0.0001

// This constant avoids the influence of fragments, which are too far away.
#define CAP_MAX_DISTANCE 0.005

uniform sampler2D u_texture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_depthTexture;

uniform vec3 u_kernel[KERNEL_SIZE];

uniform sampler2D u_rotationNoiseTexture; 

uniform vec2 u_rotationNoiseScale;

uniform mat4 u_inverseProjectionMatrix;
uniform mat4 u_projectionMatrix;

uniform float u_radius;

in vec2 v_texCoord;

out vec4 fragColor;

vec4 getViewPos(vec2 texCoord)
{
	// Calculate out of the fragment in screen space the view space position.

	float x = texCoord.s * 2.0 - 1.0;
	float y = texCoord.t * 2.0 - 1.0;
	
	// Assume we have a normal depth range between 0.0 and 1.0
	float z = texture(u_depthTexture, texCoord).r * 2.0 - 1.0;
	
	vec4 posProj = vec4(x, y, z, 1.0);
	
	vec4 posView = u_inverseProjectionMatrix * posProj;
	
	posView /= posView.w;
	
	return posView;
}

void main(void)
{
	// Calculate out of the current fragment in screen space the view space position.
	
	vec4 posView = getViewPos(v_texCoord);
	
	// Normal gathering.
	
	vec3 normalView = normalize(texture(u_normalTexture, v_texCoord).xyz * 2.0 - 1.0);
	
	// Calculate the rotation matrix for the kernel.
	
	vec3 randomVector = normalize(texture(u_rotationNoiseTexture, v_texCoord * u_rotationNoiseScale).xyz * 2.0 - 1.0);
	
	// Using Gram-Schmidt process to get an orthogonal vector to the normal vector.
	// The resulting tangent is on the same plane as the random and normal vector. 
	// see http://en.wikipedia.org/wiki/Gram%E2%80%93Schmidt_process
	// Note: No division by <u,u> needed, as this is for normal vectors 1. 
	vec3 tangentView = normalize(randomVector - dot(randomVector, normalView) * normalView);
	
	vec3 bitangentView = cross(normalView, tangentView);
	
	// Final matrix to reorient the kernel depending on the normal and the random vector.
	mat3 kernelMatrix = mat3(tangentView, bitangentView, normalView); 

	// Go through the kernel samples and create occlusion factor.	
	float occlusion = 0.0;
	
	for (int i = 0; i < KERNEL_SIZE; i++)
	{
		// Reorient sample vector in view space ...
		vec3 sampleVectorView = kernelMatrix * u_kernel[i];
		
		// ... and calculate sample point.
		vec4 samplePointView = posView + u_radius * vec4(sampleVectorView, 0.0);
		
		// Project point and calculate NDC.
		
		vec4 samplePointNDC = u_projectionMatrix * samplePointView;
		
		samplePointNDC /= samplePointNDC.w;
		
		// Create texture coordinate out of it.
		
		vec2 samplePointTexCoord = samplePointNDC.xy * 0.5 + 0.5;   
		
		// Get sample out of depth texture

		float zSceneNDC = texture(u_depthTexture, samplePointTexCoord).r * 2.0 - 1.0;
		
		float delta = samplePointNDC.z - zSceneNDC;
		
		// If scene fragment is before (smaller in z) sample point, increase occlusion.
		if (delta > CAP_MIN_DISTANCE && delta < CAP_MAX_DISTANCE)
		{
			occlusion += 1.0;
		}
	}
	
	// No occlusion gets white, full occlusion gets black.
	occlusion = 1.0 - occlusion / (float(KERNEL_SIZE) - 1.0);

	fragColor = vec4(occlusion, occlusion, occlusion, 1.0);
}
