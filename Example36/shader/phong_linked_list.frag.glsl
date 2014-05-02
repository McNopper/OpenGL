#version 440 core

#define BINDING_ATOMIC_FREE_INDEX 0
#define BINDING_IMAGE_HEAD_INDEX 0
#define BINDING_BUFFER_LINKED_LIST 0

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

struct NodeType
{
	vec4 color;
	float depth;
	uint nextIndex;
};

uniform	LightProperties u_light;
uniform	MaterialProperties u_material;

layout(binding = BINDING_ATOMIC_FREE_INDEX) uniform atomic_uint u_freeNodeIndex;

layout(binding = BINDING_IMAGE_HEAD_INDEX, r32ui) uniform uimage2D u_headIndex;

layout(binding = BINDING_BUFFER_LINKED_LIST, std430) buffer LinkedList
{
	NodeType nodes[];
	// Padding[]
};

uniform uint u_maxNodes;

in vec3 v_normal;
in vec3 v_eye;

void main()
{
	// Get the index of the next free node in the buffer.
	uint freeNodeIndex = atomicCounterIncrement(u_freeNodeIndex);

	// Check, if still space in the buffer.
	if(freeNodeIndex < u_maxNodes)
	{
		// Replace new index as the new head and gather the previous head, which will be the next index of this entry.
		uint nextIndex = imageAtomicExchange(u_headIndex, ivec2(gl_FragCoord.xy), freeNodeIndex);
		
		// Calculate color.

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

		//
		
		// Store the color, depth and the next index for later resolving.
		nodes[freeNodeIndex].color = vec4(color.rgb, u_material.alpha);
		nodes[freeNodeIndex].depth = gl_FragCoord.z;
		nodes[freeNodeIndex].nextIndex = nextIndex;
  	} 

	// No output to the framebuffer.
}   