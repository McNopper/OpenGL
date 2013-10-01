#version 430 core

#define MAX_DEPTH 5

#define NUM_SPHERES 6
#define NUM_LIGHTS 1

#define INFINITY 1000000.0

// see g_localSize = 16 in main.c. Also the division by 16.0 depends on this size.
layout (local_size_x = 16, local_size_y = 16) in;

layout (binding = 0, rgba8) uniform image2D u_texture; 

layout (std430, binding = 1) buffer Directions
{
	vec3 direction[];
	// Padding
} b_directions;

layout (std430, binding = 2) buffer Positions
{
	vec4 position[];
} b_positions;

struct Ray {
	vec4 position;
	vec3 direction;
	float valid;
};

layout (std430, binding = 3) buffer RayStacks
{
	Ray ray[];
} b_rayStacks;

//

struct Material {
	vec4 emissiveColor;

	vec4 diffuseColor;

	vec4 specularColor;

	float shininess;

	float alpha;

	float reflectivity;

	float padding;
};

struct Sphere
{
	vec4 center;

	float radius;

	vec3 padding;

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

layout (std430, binding = 6) buffer Colors
{
	vec4 color[];
} b_colors;

//

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

void trace(int rayIndex, int maxLoops, int pixelPos)
{
	if (b_rayStacks.ray[pixelPos * maxLoops + rayIndex].valid <= 0.0)
	{
		return;
	}
	
	const float bias = 1e-4;

	int i, k;

	float tNear = INFINITY;
	int sphereNearIndex = -1;
	bool insideSphereNear = false;

	vec3 ray;

	vec4 hitPosition;
	vec3 hitDirection;

	vec4 biasedPositiveHitPosition;
	vec4 biasedNegativeHitPosition;
	vec3 biasedHitDirection;

	vec3 eyeDirection;

	//

	for (i = 0; i < NUM_SPHERES; i++)
	{
		float t0 = INFINITY;
		float t1 = INFINITY;
		bool insideSphere = false;

		int numberIntersections = intersectRaySphere(t0, t1, insideSphere, b_rayStacks.ray[pixelPos * maxLoops + rayIndex].position, b_rayStacks.ray[pixelPos * maxLoops + rayIndex].direction, b_spheres.sphere[i].center, b_spheres.sphere[i].radius);

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
		b_colors.color[pixelPos * maxLoops + rayIndex] = vec4(0.8, 0.8, 0.8, 1.0);
	
		return;
	}	
	
	// TODO For now and debugging, just a red color.
	b_colors.color[pixelPos * maxLoops + rayIndex] = vec4(1.0, 0.0, 0.0, 1.0);
	
	// TODO Do checks similar as in Example29

	// TODO If reflection:
	// b_rayStacks.ray[getReflectIndex(rayIndex)].valid = 1.0;
	// Store position and direction
	
	// TODO If refraction:
	// b_rayStacks.ray[getRefractIndex(rayIndex)].valid = 1.0;
	// Store position and direction
	
	// TODO Store hit sphere. 
}

void shade(int rayIndex, int maxLoops, int pixelPos)
{
	// TODO Calcualte final color.
}

void clear(int rayIndex, int maxLoops, int pixelPos)
{
	b_rayStacks.ray[pixelPos * maxLoops + rayIndex].valid = 0.0;
}

void main(void)
{
	ivec2 dimension = imageSize(u_texture);

	ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

	int pixelPos = storePos.x + storePos.y * dimension.x;

	// Maximum possible traces	
	int maxLoops = int(exp2(MAX_DEPTH) - 1.0);

	// Init ray stack 0 with initial direction and position.
	b_rayStacks.ray[pixelPos * maxLoops + 0].position = b_positions.position[pixelPos];
	b_rayStacks.ray[pixelPos * maxLoops + 0].direction = b_directions.direction[pixelPos];
	b_rayStacks.ray[pixelPos * maxLoops + 0].valid = 1.0;

	// Trace all possible rays initiated by the origin ray.
	for (int i = 0; i < maxLoops; i++)
	{
		trace(i, maxLoops, pixelPos);
	}
		
	// Loop from end again and calculate final color.
	for (int i = maxLoops - 1; i >= 0; i--)
	{
		shade(i, maxLoops, pixelPos);
	}	

	// Clear for next frame.
	for (int i = 0; i < maxLoops; i++)
	{
		clear(i, maxLoops, pixelPos);
	}

	imageStore(u_texture, storePos, b_colors.color[pixelPos * maxLoops + 0]);	
}
