#version 430 core

// see same define in main.c
#define N 512
#define GLUS_PI	3.1415926535897932384626433832795

uniform int u_processColumn;

uniform int u_steps;

layout (binding = 0, rg32f) uniform image2D u_imageIn; 
layout (binding = 1, rg32f) uniform image2D u_imageOut;

// FIXME: On AMD hardware, an integer texture can not be created.
// Note: If a shader storage buffer is used, the driver on AMD hardware crashes.
//		 If a uniform int array is used and N >= 512, the shader linker on NVIDIA hardware (GT640, Linux)
//       reports an internal error and exits.
layout (binding = 2, r32f) uniform image1D u_imageIndices;

// as N = 512, so local size is 512/2 = 256. Processing two fields per invocation.
layout (local_size_x = 256, local_size_y = 1, local_size_z = 1) in;

vec2 mulc(vec2 a, vec2 b)
{
	vec2 result;
	
	result.x = a.x * b.x - a.y * b.y;
	result.y = a.x * b.y + b.x * a.y;
	
	return result;
}

vec2 rootOfUnityc(int n, int k)
{
	vec2 result;
	
	result.x = cos(2.0 * GLUS_PI * float(k) / float(n));
	result.y = sin(2.0 * GLUS_PI * float(k) / float(n));

	return result;
}

void main(void)
{
	ivec2 leftLoadPos;
	ivec2 rightLoadPos;

	ivec2 leftStorePos;
	ivec2 rightStorePos;

	int leftIndex = int(imageLoad(u_imageIndices, 2 * int(gl_GlobalInvocationID.x)).r);
	int rightIndex = int(imageLoad(u_imageIndices, 2 * int(gl_GlobalInvocationID.x) + 1).r);

	if (u_processColumn == 0)
	{
		leftLoadPos = ivec2(leftIndex, int(gl_GlobalInvocationID.y));
		rightLoadPos = ivec2(rightIndex, int(gl_GlobalInvocationID.y));

		leftStorePos = ivec2(2 * int(gl_GlobalInvocationID.x), int(gl_GlobalInvocationID.y));
		rightStorePos = ivec2(2 * int(gl_GlobalInvocationID.x) + 1, int(gl_GlobalInvocationID.y));
	}
	else
	{
		leftLoadPos = ivec2(int(gl_GlobalInvocationID.y), leftIndex);
		rightLoadPos = ivec2(int(gl_GlobalInvocationID.y), rightIndex);

		leftStorePos = ivec2(int(gl_GlobalInvocationID.y), 2 * int(gl_GlobalInvocationID.x));
		rightStorePos = ivec2(int(gl_GlobalInvocationID.y), 2 * int(gl_GlobalInvocationID.x) + 1);
	}

	// Copy and swizzle values for butterfly algortihm.
	vec2 leftValue = imageLoad(u_imageIn, leftLoadPos).xy;
	vec2 rightValue = imageLoad(u_imageIn, rightLoadPos).xy;

	imageStore(u_imageOut, leftStorePos, vec4(leftValue, 0.0, 0.0));
	imageStore(u_imageOut, rightStorePos, vec4(rightValue, 0.0, 0.0));

	// Make sure that all values are stored and visible after the barrier. 
	memoryBarrier();	
	barrier();
	
	//

	int numberSections = N / 2;
	int numberButterfliesInSection = 1;

	int currentSection = int(gl_GlobalInvocationID.x);
	int currentButterfly = 0;

	// Performing needed FFT steps per either row or column.
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
	
		// "Butterfly" math.
		
		// Note: To improve performance, put root of unity calculation in look up texture or buffer. 		 						
		vec2 currentW = rootOfUnityc(numberButterfliesInSection * 2, currentButterfly);
	
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

		// Change parameters for butterfly and section index calculation.		
		numberButterfliesInSection *= 2;
		numberSections /= 2;

		currentSection /= 2;
		currentButterfly = int(gl_GlobalInvocationID.x) % numberButterfliesInSection;

		// Make sure, that all shaders are at the same stage, as now indices are changed.
		barrier();
	}
	
	// Process twiddle factor for second pass FFT. 
	if (u_processColumn == 1)
	{
		if ((leftStorePos.x + leftStorePos.y) % 2 == 0)
		{
			leftValue = imageLoad(u_imageOut, leftStorePos).xy;
			
			leftValue.x *= -1.0;
			
			imageStore(u_imageOut, leftStorePos, vec4(leftValue, 0.0, 0.0));		
		}
		if ((rightStorePos.x + rightStorePos.y) % 2 == 0)
		{
			rightValue = imageLoad(u_imageOut, rightStorePos).xy;

			rightValue.x *= -1.0;
			
			imageStore(u_imageOut, rightStorePos, vec4(rightValue, 0.0, 0.0));
		}
	}
}
