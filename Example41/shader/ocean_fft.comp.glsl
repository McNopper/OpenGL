#version 430 core

#define GLUS_PI	3.1415926535897932384626433832795

// see same define in main.c
#define N 512

uniform int u_processColumn;

uniform int u_steps;

uniform int u_indices[N];

layout (binding = 0, rg32f) uniform image2D u_imageIn; 
layout (binding = 1, rg32f) uniform image2D u_imageOut;

// as N = 512, so local size is 512/2 = 256
layout (local_size_x = 256) in;

vec2 expc(vec2 c)
{
	vec2 result;

	result.x = exp(c.x) * cos(c.y);
	result.y = exp(c.x) * sin(c.y);
	
	return result;
}

vec2 mulc(vec2 a, vec2 b)
{
	vec2 result;
	
	result.x = a.x * b.x - a.y * b.y;
	result.y = a.x * b.y + b.x * a.y;
	
	return result;
}

vec2 rootOfUnityc(int n, int k)
{
	vec2 exponent;
	exponent.x = 0.0;
	exponent.y = 2.0 * GLUS_PI * float(k) / float(n);

	return expc(exponent);
}

void main(void)
{
	ivec2 leftLoadPos;
	ivec2 rightLoadPos;

	ivec2 leftStorePos;
	ivec2 rightStorePos;

	if (u_processColumn == 0)
	{
		leftLoadPos = ivec2(u_indices[2 * int(gl_GlobalInvocationID.x)], int(gl_GlobalInvocationID.y));
		rightLoadPos = ivec2(u_indices[2 * int(gl_GlobalInvocationID.x) + 1], int(gl_GlobalInvocationID.y));

		leftStorePos = ivec2(2 * int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.y));
		rightStorePos = ivec2(2 * int(gl_GlobalInvocationID.x) + 1, int(gl_GlobalInvocationID.y));
	}
	else
	{
		leftLoadPos = ivec2(int(gl_GlobalInvocationID.y), u_indices[2 * int(gl_GlobalInvocationID.x)]);
		rightLoadPos = ivec2(int(gl_GlobalInvocationID.y), u_indices[2 * int(gl_GlobalInvocationID.x) + 1]);

		leftStorePos = ivec2(int(gl_GlobalInvocationID.y), 2 * int(gl_GlobalInvocationID.x));
		rightStorePos = ivec2(int(gl_GlobalInvocationID.y), 2 * int(gl_GlobalInvocationID.x) + 1);
	}

	vec2 leftValue = imageLoad(u_imageIn, leftLoadPos).xy;
	vec2 rightValue = imageLoad(u_imageIn, rightLoadPos).xy;

	imageStore(u_imageOut, leftStorePos, vec4(leftValue, 0.0, 0.0));
	imageStore(u_imageOut, rightStorePos, vec4(rightValue, 0.0, 0.0));

	memoryBarrier();
	
	barrier();
	
	//

	int numberSections = N / 2;
	int numberButterfliesInSection = 1;

	int currentSection = int(gl_GlobalInvocationID.x);
	int currentButterfly = 0;

	for (int currentStep = 0; currentStep < u_steps; currentStep++)
	{	
		int leftIndex = currentButterfly + currentSection * numberButterfliesInSection * 2;
		int rightIndex = currentButterfly + numberButterfliesInSection + currentSection * numberButterfliesInSection * 2;
	
		if (u_processColumn == 0)
		{
			leftStorePos = ivec2(leftIndex, int(gl_GlobalInvocationID.y));
			rightStorePos = ivec2(rightIndex, int(gl_GlobalInvocationID.y));
		}
		else
		{
			leftStorePos = ivec2(int(gl_GlobalInvocationID.y), leftIndex);
			rightStorePos = ivec2(int(gl_GlobalInvocationID.y), rightIndex);
		}
	
		leftValue = imageLoad(u_imageOut, leftStorePos).xy;
		rightValue = imageLoad(u_imageOut, rightStorePos).xy;
	
		//
						
		vec2 currentW = rootOfUnityc(numberButterfliesInSection * 2, -currentButterfly);
	
		vec2 multiply;
		vec2 addition;
		vec2 subtraction;

		multiply = mulc(currentW, rightValue);	
		
		addition = leftValue + multiply;
		subtraction = leftValue - multiply; 

		imageStore(u_imageOut, leftStorePos, vec4(addition, 0.0, 0.0));
		imageStore(u_imageOut, rightStorePos, vec4(subtraction, 0.0, 0.0));

		// Make sure, that values are written.		
		memoryBarrier();

		// Change conditions for index calculation.
		
		numberButterfliesInSection *= 2;
		numberSections /= 2;

		currentSection /= 2;
		currentButterfly = int(gl_GlobalInvocationID.x) % numberButterfliesInSection;

		// Make sure, that all shaders are at the same stage, as now indices are changed.
		barrier();
	}
}
