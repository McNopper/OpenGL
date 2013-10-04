#version 430 core

// Indices of refraction
#define Air 1.0
#define Bubble 1.06

// Air to glass ratio of the indices of refraction (Eta)
#define Eta (Air / Bubble)

// see http://en.wikipedia.org/wiki/Refractive_index Reflectivity
#define R0 (((Air - Bubble) * (Air - Bubble)) / ((Air + Bubble) * (Air + Bubble)))

// Adjust NUM_STACK_NODES in C file, if this value is changed.
#define MAX_DEPTH 5

#define NUM_SPHERES 6
#define NUM_LIGHTS 1

#define INFINITY 1000000.0

// see g_localSize = 16 in main.c.
layout (local_size_x = 16, local_size_y = 16) in;

layout (rgba8, binding = 0) uniform image2D u_texture; 

layout (std430, binding = 1) buffer Directions
{
	vec3 direction[];
	// Padding[]
} b_directions;

layout (std430, binding = 2) buffer Positions
{
	vec4 position[];
} b_positions;

struct Stack {
	vec4 position;
	
	vec3 direction;

	vec4 color;
	
	vec4 hitPosition;
	
	vec3 hitDirection;

	vec4 biasedPositiveHitPosition;
	
	float valid;

	float fresnel;

	float sphereIndex;
	
	float insideSphere;
};

layout (std430, binding = 3) buffer Stacks
{
	Stack stack[];
} b_stacks;

//

struct Material {
	vec4 emissiveColor;

	vec4 diffuseColor;

	vec4 specularColor;

	float shininess;

	float alpha;

	float reflectivity;

	// Padding
};

struct Sphere
{
	vec4 center;

	float radius;

	// Padding[3]

	Material material;
};

layout (std430, binding = 4) buffer Spheres
{
	Sphere sphere[];
} b_spheres;

//

struct PointLight
{
	vec4 position;

	vec4 color;
};

layout (std430, binding = 5) buffer PointLights
{
	PointLight pointLight[];
} b_pointLights;

//

float fresnel(vec3 incident, vec3 normal, float r0)
{
	// see http://en.wikipedia.org/wiki/Schlick%27s_approximation

	return r0 + (1.0 - r0) * pow(1.0 - dot(-incident, normal), 5.0);
}

int intersectRaySphere(out float tNear, out float tFar, out bool insideSphere, vec4 rayStart, vec3 rayDirection, vec4 sphereCenter, float radius)
{
	// see http://de.wikipedia.org/wiki/Quadratische_Gleichung (German)
	// see Real-Time Collision Detection p177

	int intersections = 0;
	vec3 m;
	float b, c, discriminant, sqrtDiscriminant, t;

	//

	m = (rayStart - sphereCenter).xyz;

	b = dot(m, rayDirection);
	c = dot(m, m) - radius * radius;

	// Ray pointing away from sphere and ray starts outside sphere.
	if (b > 0.0 && c > 0.0)
	{
		return intersections;
	}

	discriminant = b * b - c;

	// No solution, so no intersection. Ray passes the sphere.
	if (discriminant < 0.0)
	{
		return intersections;
	}

	sqrtDiscriminant = sqrt(discriminant);

	// If we come so far, we have at least one intersection.

	insideSphere = false;

	t = -b - sqrtDiscriminant;

	// Ray starts inside the sphere, as t is negative.
	if (t < 0.0)
	{
		// Use origin as intersection point.
		t = 0.0;

		insideSphere = true;
	}

	intersections++;

	tNear = t;

	// Tangent point. Only one intersection. So leave.
	if (discriminant == 0.0)
	{
		// Save in both for convenience.
		tFar = t;

		return intersections;
	}

	//

	t = -b + sqrtDiscriminant;

	intersections++;

	tFar = t;

	return intersections;
}

int getReflectIndex(int index)
{
	return index * 2 + 1;
}

int getRefractIndex(int index)
{
	return index * 2 + 2;
}

void trace(int pixelPos, int maxLoops, int rayIndex)
{
	if (b_stacks.stack[pixelPos * maxLoops + rayIndex].valid <= 0.0)
	{
		return;
	}
	
	const float bias = 1e-4;

	int i, k;

	float tNear = INFINITY;
	int sphereNearIndex = -1;
	bool insideSphereNear = false;

	vec3 ray;

	//
	
	vec4 rayPosition = b_stacks.stack[pixelPos * maxLoops + rayIndex].position;
	
	vec3 rayDirection = b_stacks.stack[pixelPos * maxLoops + rayIndex].direction;

	for (i = 0; i < NUM_SPHERES; i++)
	{
		float t0 = INFINITY;
		float t1 = INFINITY;
		bool insideSphere = false;

		int numberIntersections = intersectRaySphere(t0, t1, insideSphere, rayPosition, rayDirection, b_spheres.sphere[i].center, b_spheres.sphere[i].radius);

		if (numberIntersections > 0)
		{
			// If intersection happened inside the sphere, take second intersection point, as this one is on the surface.
			if (insideSphere)
			{
				t0 = t1;
			}

			// Found a sphere, which is closer.
			if (t0 < tNear)
			{
				tNear = t0;
				sphereNearIndex = i;
				insideSphereNear = insideSphere;			
			}
		}
	}

	//

	// No intersection, return background color / ambient light.
	if (sphereNearIndex < 0)
	{
		b_stacks.stack[pixelPos * maxLoops + rayIndex].sphereIndex = -1.0;
	
		return;
	}
	
	b_stacks.stack[pixelPos * maxLoops + rayIndex].sphereIndex = float(sphereNearIndex);	
	b_stacks.stack[pixelPos * maxLoops + rayIndex].insideSphere = float(insideSphereNear);
	
	//
	
	// Calculate ray hit position ...
	ray = rayDirection * tNear;
	vec4 hitPosition = rayPosition + vec4(ray, 0.0);

	// ... and normal
	vec3 hitDirection = normalize((hitPosition - b_spheres.sphere[sphereNearIndex].center).xyz);

	// If inside the sphere, reverse hit vector, as ray comes from inside.
	if (insideSphereNear)
	{
		hitDirection *= -1.0;
	}

	//

	// Biasing, to avoid artifacts.
	vec3 biasedHitDirection = hitDirection * bias;
	vec4 biasedPositiveHitPosition = hitPosition + vec4(biasedHitDirection, 0.0);
	vec4 biasedNegativeHitPosition = hitPosition - vec4(biasedHitDirection, 0.0);

	//

	b_stacks.stack[pixelPos * maxLoops + rayIndex].fresnel = fresnel(rayDirection, hitDirection, R0);
	
	int reflectionIndex = getReflectIndex(rayIndex);

	// Reflection ...
	if (b_spheres.sphere[sphereNearIndex].material.reflectivity > 0.0 && reflectionIndex < maxLoops)
	{
		vec3 reflectionDirection = normalize(reflect(rayDirection, hitDirection));

		b_stacks.stack[pixelPos * maxLoops + reflectionIndex].position = biasedPositiveHitPosition;
		b_stacks.stack[pixelPos * maxLoops + reflectionIndex].direction = reflectionDirection;
		b_stacks.stack[pixelPos * maxLoops + reflectionIndex].valid = 1.0;
	}

	int refractionIndex = getRefractIndex(rayIndex);

	// ... refraction.
	if (b_spheres.sphere[sphereNearIndex].material.alpha < 1.0 && refractionIndex < maxLoops)
	{
		// If inside, it is from glass to air.
		float eta = insideSphereNear ? 1.0 / Eta : Eta;
		
		vec3 refractionDirection = normalize(refract(rayDirection, hitDirection, eta));

		b_stacks.stack[pixelPos * maxLoops + refractionIndex].position = biasedNegativeHitPosition;
		b_stacks.stack[pixelPos * maxLoops + refractionIndex].direction = refractionDirection;
		b_stacks.stack[pixelPos * maxLoops + refractionIndex].valid = 1.0;
	}
	else
	{
		b_stacks.stack[pixelPos * maxLoops + rayIndex].fresnel = 1.0;
	}
	
	b_stacks.stack[pixelPos * maxLoops + rayIndex].hitPosition = hitPosition;
	b_stacks.stack[pixelPos * maxLoops + rayIndex].hitDirection = hitDirection; 

	b_stacks.stack[pixelPos * maxLoops + rayIndex].biasedPositiveHitPosition = biasedPositiveHitPosition;
}

void shade(int pixelPos, int maxLoops, int rayIndex)
{
	int i, k;

	if (b_stacks.stack[pixelPos * maxLoops + rayIndex].valid <= 0.0)
	{
		return;
	}
	
	int sphereNearIndex = int(b_stacks.stack[pixelPos * maxLoops + rayIndex].sphereIndex);
	
	if (sphereNearIndex < 0)
	{
		b_stacks.stack[pixelPos * maxLoops + rayIndex].color = vec4(0.8, 0.8, 0.8, 1.0);
		
		return;
	}	

	bool insideSphereNear = bool(b_stacks.stack[pixelPos * maxLoops + rayIndex].insideSphere);

	vec4 rayPosition = b_stacks.stack[pixelPos * maxLoops + rayIndex].position;
	vec3 rayDirection = b_stacks.stack[pixelPos * maxLoops + rayIndex].direction;

	vec4 reflectionColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 refractionColor = vec4(0.0, 0.0, 0.0, 1.0);

	int reflectionIndex = getReflectIndex(rayIndex);

	float fresnel = b_stacks.stack[pixelPos * maxLoops + rayIndex].fresnel;
	
	// Reflection ...
	if (b_spheres.sphere[sphereNearIndex].material.reflectivity > 0.0 && reflectionIndex < maxLoops)
	{
		if (b_stacks.stack[pixelPos * maxLoops + reflectionIndex].valid > 0.0)
		{
			reflectionColor = b_stacks.stack[pixelPos * maxLoops + reflectionIndex].color;
		}
	}

	int refractionIndex = getRefractIndex(rayIndex);

	// ... refraction.
	if (b_spheres.sphere[sphereNearIndex].material.alpha < 1.0 && refractionIndex < maxLoops)
	{
		if (b_stacks.stack[pixelPos * maxLoops + refractionIndex].valid > 0.0)
		{
			refractionColor = b_stacks.stack[pixelPos * maxLoops + refractionIndex].color;
		}
	}

	//

	vec3 eyeDirection = rayDirection * -1.0;

	vec4 biasedPositiveHitPosition = b_stacks.stack[pixelPos * maxLoops + rayIndex].biasedPositiveHitPosition;
	vec3 hitDirection = b_stacks.stack[pixelPos * maxLoops + rayIndex].hitDirection;

	vec4 pixelColor = vec4(0.0, 0.0, 0.0, 1.0);

	// Diffuse and specular color
	for (i = 0; i < NUM_LIGHTS; i++)
	{
		bool obstacle = false;

		vec3 lightDirection = normalize((b_pointLights.pointLight[i].position - b_stacks.stack[pixelPos * maxLoops + rayIndex].hitPosition).xyz); 
		vec3 incidentLightDirection = lightDirection * -1.0; 

		// Check for obstacles between current hit point surface and point light.
		for (k = 0; k < NUM_SPHERES; k++)
		{
			if (k == sphereNearIndex)
			{
				continue;
			}
			
			float t0 = INFINITY;
			float t1 = INFINITY;
			bool insideSphere = false;
			
			int numberIntersections = intersectRaySphere(t0, t1, insideSphere, biasedPositiveHitPosition, lightDirection, b_spheres.sphere[k].center, b_spheres.sphere[k].radius);

			if (numberIntersections > 0)
			{
				obstacle = true;

				break;
			}
		}

		// If no obstacle, illuminate hit point surface.
		if (!obstacle)
		{
			float diffuseIntensity = max(0.0, dot(hitDirection, lightDirection));

			if (diffuseIntensity > 0.0)
			{
				pixelColor += diffuseIntensity * b_spheres.sphere[sphereNearIndex].material.diffuseColor * b_pointLights.pointLight[i].color;

				vec3 specularReflection = normalize(reflect(incidentLightDirection, hitDirection));

				float eDotR = max(0.0, dot(eyeDirection, specularReflection));

				if (eDotR > 0.0 && !insideSphereNear)
				{
					float specularIntensity = pow(eDotR, b_spheres.sphere[sphereNearIndex].material.shininess);

					pixelColor += specularIntensity * b_spheres.sphere[sphereNearIndex].material.specularColor * b_pointLights.pointLight[i].color;
				}
			}
		}
	}

	// Emissive color
	pixelColor += b_spheres.sphere[sphereNearIndex].material.emissiveColor;

	// Final color with reflection and refraction
	pixelColor = (1.0 - fresnel) * refractionColor * (1.0 - b_spheres.sphere[sphereNearIndex].material.alpha) + pixelColor * (1.0 - b_spheres.sphere[sphereNearIndex].material.reflectivity) * b_spheres.sphere[sphereNearIndex].material.alpha + fresnel * reflectionColor * b_spheres.sphere[sphereNearIndex].material.reflectivity;  
	
	b_stacks.stack[pixelPos * maxLoops + rayIndex].color = pixelColor;
}

void clear(int pixelPos, int maxLoops, int rayIndex)
{
	b_stacks.stack[pixelPos * maxLoops + rayIndex].valid = 0.0;
}

void main(void)
{
	ivec2 dimension = imageSize(u_texture);

	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

	int pixelPos = storePos.x + storePos.y * dimension.x;

	// Maximum possible traces	
	int maxLoops = int(exp2(MAX_DEPTH) - 1.0);

	// Init ray stack 0 with initial direction and position.
	b_stacks.stack[pixelPos * maxLoops + 0].position = b_positions.position[pixelPos];
	b_stacks.stack[pixelPos * maxLoops + 0].direction = b_directions.direction[pixelPos];
	b_stacks.stack[pixelPos * maxLoops + 0].valid = 1.0;

	// Trace all possible rays initiated by the origin ray.
	for (int i = 0; i < maxLoops; i++)
	{
		trace(pixelPos, maxLoops, i);
	}
		
	// Loop from leaf nodes to root and calculate final color.
	for (int i = maxLoops - 1; i >= 0; i--)
	{
		shade(pixelPos, maxLoops, i);
	}	

	// Clear for next frame.
	for (int i = 0; i < maxLoops; i++)
	{
		clear(pixelPos, maxLoops, i);
	}

	// In index 0, the final color for this pixel is stored.
	imageStore(u_texture, storePos, b_stacks.stack[pixelPos * maxLoops + 0].color);	
}
