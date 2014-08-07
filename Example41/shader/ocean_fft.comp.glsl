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

// Faster, when stored in shared memory compared to global memory.
shared vec2 sharedStore[N];

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
	ivec2 leftStorePos;
	ivec2 rightStorePos;

	ivec2 leftLoadPos;
	ivec2 rightLoadPos;

	int xIndex = int(gl_GlobalInvocationID.x);
	int yIndex = int(gl_GlobalInvocationID.y);

	int leftStoreIndex = 2 * xIndex;
	int rightStoreIndex = 2 * xIndex + 1;

	// Load the swizzled indices.
	int leftLoadIndex = int(imageLoad(u_imageIndices, leftStoreIndex).r);
	int rightLoadIndex = int(imageLoad(u_imageIndices, rightStoreIndex).r);

	// Loading and storing position depends on processing per row or column.
	if (u_processColumn == 0)
	{
		leftLoadPos = ivec2(leftLoadIndex, yIndex);
		rightLoadPos = ivec2(rightLoadIndex, yIndex);

		leftStorePos = ivec2(leftStoreIndex, yIndex);
		rightStorePos = ivec2(rightStoreIndex, yIndex);
	}
	else
	{
		leftLoadPos = ivec2(yIndex, leftLoadIndex);
		rightLoadPos = ivec2(yIndex, rightLoadIndex);

		leftStorePos = ivec2(yIndex, leftStoreIndex);
		rightStorePos = ivec2(yIndex, rightStoreIndex);
	}

	// Copy and swizzle values for butterfly algortihm into the shared memory.
	vec2 leftValue = imageLoad(u_imageIn, leftLoadPos).xy;
	vec2 rightValue = imageLoad(u_imageIn, rightLoadPos).xy;

	sharedStore[leftStoreIndex] = leftValue;
	sharedStore[rightStoreIndex] = rightValue;

	// Make sure that all values are stored and visible after the barrier. 
	memoryBarrierShared();
	barrier();
	
	//
	
	int numberSections = N / 2;
	int numberButterfliesInSection = 1;

	int currentSection = xIndex;
	int currentButterfly = 0;

	// Performing needed FFT steps per either row or column.
	for (int currentStep = 0; currentStep < u_steps; currentStep++)
	{	
		int leftIndex = currentButterfly + currentSection * numberButterfliesInSection * 2;
		int rightIndex = currentButterfly + numberButterfliesInSection + currentSection * numberButterfliesInSection * 2;
	
		leftValue = sharedStore[leftIndex];
		rightValue = sharedStore[rightIndex];
	
		// "Butterfly" math.
		
		// Note: To improve performance, put root of unity calculation in look up texture or buffer. 		 						
		vec2 currentW = rootOfUnityc(numberButterfliesInSection * 2, currentButterfly);
	
		vec2 multiply;
		vec2 addition;
		vec2 subtraction;

		multiply = mulc(currentW, rightValue);	
		
		addition = leftValue + multiply;
		subtraction = leftValue - multiply; 

		sharedStore[leftIndex] = addition;
		sharedStore[rightIndex] = subtraction;		

		// Make sure, that values are written.		
		memoryBarrierShared();

		// Change parameters for butterfly and section index calculation.		
		numberButterfliesInSection *= 2;
		numberSections /= 2;

		currentSection /= 2;
		currentButterfly = xIndex % numberButterfliesInSection;

		// Make sure, that all shaders are at the same stage, as now indices are changed.
		barrier();
	}
	
	// Process twiddle factor for second pass FFT. 
	if (u_processColumn == 1)
	{
		if ((leftStorePos.x + leftStorePos.y) % 2 == 0)
		{
			sharedStore[leftStoreIndex] *= -1.0;
		}
		if ((rightStorePos.x + rightStorePos.y) % 2 == 0)
		{
			sharedStore[rightStoreIndex] *= -1.0;			
		}
		
		// Make sure, that values are written.		
		memoryBarrierShared();
	}
	
	// Store from shared to global memory.
	imageStore(u_imageOut, leftStorePos, vec4(sharedStore[leftStoreIndex], 0.0, 0.0));
	imageStore(u_imageOut, rightStorePos, vec4(sharedStore[rightStoreIndex], 0.0, 0.0));
}
