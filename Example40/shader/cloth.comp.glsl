#version 430 core

// Number of iterations the spring constraints and collisions are updated.
#define ITERATIONS 4

// Unit: kg * m * s*s
#define FORCE vec4(0.0, -0.2, 0.0, 0.0)

// Unit: kg
#define MASS 0.5

// Offset, that a particle is not inside the sphere surface.
#define RADIUS_TOLERANCE 0.01

uniform int u_verticesPerRow;

uniform float u_deltaTime;

// Distance at rest between vertical and horizontal particle.
uniform float u_distanceRest;

// One coliding sphere.
uniform vec4 u_sphereCenter;
uniform float u_sphereRadius;

layout(std430, binding=1) buffer VertexInPrevious {
    vec4 b_vertexInPrevious[];
};

layout(std430, binding=2) buffer VertexInCurrent {
    vec4 b_vertexInCurrent[];
};

layout(std430, binding=3) buffer VertexOut {
    vec4 b_vertexOut[];
};

layout(std430, binding=4) buffer NormalOut {
    vec3 b_normalOut[];
    // Padding[]
};

// (ROWS+1)^2
layout (local_size_x = 1024) in;

void calculateDeltaVector(out vec4 deltaVector, vec4 a, vec4 b)
{
	vec4 vector = a - b;
	
	float deltaLength = length(vector);

	// 0.5 because this happens on each side.
	deltaVector = vector * 0.5 * (u_distanceRest - deltaLength) / deltaLength;
}

// see http://web.archive.org/web/20070610223835/http://www.teknikus.dk/tj/gdc2001.htm
//     http://developer.download.nvidia.com/SDK/10/direct3d/Source/Cloth/doc/Cloth.pdf
void main(void)
{
    int currentIndex = int(gl_GlobalInvocationID.x) + int(gl_GlobalInvocationID.y) * u_verticesPerRow;

    int currentIndexRow = currentIndex % u_verticesPerRow;
    int currentIndexColumn = currentIndex / u_verticesPerRow;

	//
	// Summing all forces, to get the acceleration. In this case, only one force directing downwards is used to simulate gravity.
	// 
	
	vec4 a = FORCE / MASS;

	//
	// Verlet integration.
	//

	float squareDeltaTime = u_deltaTime * u_deltaTime;
		
	vec4 x = b_vertexInCurrent[currentIndex];
	 
	vec4 xPrev = b_vertexInPrevious[currentIndex];

    b_vertexOut[currentIndex] = 2.0 * x - xPrev + a * squareDeltaTime;
    
    memoryBarrierBuffer();
    
	//
	// Relaxation iterations.
	//
	for (int iteration = 0; iteration < ITERATIONS; iteration++)
	{
	    //
	    // Process spring constraints.
	    //
	    
		// Alternating "even" and "odd" cells to avoid dependencies.  
		for (int evenOdd = 0; evenOdd < 2; evenOdd++)
		{
			int rightIndex;
			int bellowIndex;
			int diagonalIndex;
			
			vec4 deltaVector;
	
			if (currentIndexRow % 2 == evenOdd && currentIndexColumn % 2 == evenOdd)
			{    
				rightIndex = currentIndex + 1;
				if (rightIndex % u_verticesPerRow > currentIndexRow)
				{
					calculateDeltaVector(deltaVector, b_vertexOut[currentIndex], b_vertexOut[rightIndex]);
				
					b_vertexOut[currentIndex] += deltaVector;
					b_vertexOut[rightIndex] -= deltaVector;
					
					memoryBarrierBuffer();  
				}    			
			
			
				bellowIndex = currentIndex + u_verticesPerRow;
				if (bellowIndex < u_verticesPerRow * u_verticesPerRow)			
				{
					calculateDeltaVector(deltaVector, b_vertexOut[currentIndex], b_vertexOut[bellowIndex]);
				
					b_vertexOut[currentIndex] += deltaVector;
					b_vertexOut[bellowIndex] -= deltaVector;
					
					memoryBarrierBuffer();  
				}
			
			
				diagonalIndex = currentIndex + 1 + u_verticesPerRow;
				if (diagonalIndex < u_verticesPerRow * u_verticesPerRow && diagonalIndex % u_verticesPerRow > currentIndexRow)
				{
					calculateDeltaVector(deltaVector, b_vertexOut[rightIndex], b_vertexOut[diagonalIndex]);
			
					b_vertexOut[rightIndex] += deltaVector;
					b_vertexOut[diagonalIndex] -= deltaVector;  
			
					memoryBarrierBuffer();
			
					calculateDeltaVector(deltaVector, b_vertexOut[bellowIndex], b_vertexOut[diagonalIndex]);
			
					b_vertexOut[bellowIndex] += deltaVector;
					b_vertexOut[diagonalIndex] -= deltaVector;
					
					memoryBarrierBuffer();  
				}
			}
			else if (evenOdd == 1 && currentIndexRow % 2 == 1 && currentIndexColumn == 0)
			{
				// First row spring, when processing odd pass. 
				
				rightIndex = currentIndex + 1;
				if (rightIndex < u_verticesPerRow)
				{
					calculateDeltaVector(deltaVector, b_vertexOut[currentIndex], b_vertexOut[rightIndex]);
			
					b_vertexOut[currentIndex] += deltaVector;
					b_vertexOut[rightIndex] -= deltaVector;
					
					memoryBarrierBuffer();  
				}
			}
			
			barrier();
		}
		
		//
		// Process collision constraints.
		//
	
		vec4 sphereVector = b_vertexOut[currentIndex] - u_sphereCenter;
		
		// If particle is inside sphere ...
		if (length(sphereVector) < u_sphereRadius + RADIUS_TOLERANCE)
		{
			// ... move it outside the sphere.
			b_vertexOut[currentIndex] = u_sphereCenter + normalize(sphereVector) * (u_sphereRadius + RADIUS_TOLERANCE); 
			
			memoryBarrierBuffer();
		}
		
		barrier();		
	}	
	
    //
    // Calculate normal.
    //
    
    vec3 normal = vec3(0.0, 0.0, 0.0);
    
	vec3 tangent;
	vec3 bitangent;

	// Taking all neighbour particles, if available, into account.
    if (currentIndexRow + 1 < u_verticesPerRow)
    {
    	tangent = normalize((b_vertexOut[currentIndex + 1] - b_vertexOut[currentIndex]).xyz);
    	
	    if (currentIndexColumn + 1 < u_verticesPerRow)
	    {
	    	bitangent = normalize((b_vertexOut[currentIndex + u_verticesPerRow] - b_vertexOut[currentIndex]).xyz);
	    	
	    	normal += normalize(cross(bitangent, tangent));
	    }    	
	    if (currentIndexColumn - 1 >= 0)
	    {
	    	bitangent = normalize((b_vertexOut[currentIndex] - b_vertexOut[currentIndex - u_verticesPerRow]).xyz);
	    	
	    	normal += normalize(cross(bitangent, tangent));
	    }    	
    }
    if (currentIndexRow - 1 >= 0)
    {
    	tangent = normalize((b_vertexOut[currentIndex] - b_vertexOut[currentIndex - 1]).xyz);
    	
	    if (currentIndexColumn + 1 < u_verticesPerRow)
	    {
	    	bitangent = normalize((b_vertexOut[currentIndex + u_verticesPerRow] - b_vertexOut[currentIndex]).xyz);
	    	
	    	normal += normalize(cross(bitangent, tangent));
	    }    	
	    if (currentIndexColumn - 1 >= 0)
	    {
	    	bitangent = normalize((b_vertexOut[currentIndex] - b_vertexOut[currentIndex - u_verticesPerRow]).xyz);
	    	
	    	normal += normalize(cross(bitangent, tangent));
	    }    	
    }
    
    //
        
    b_normalOut[currentIndex] = normalize(normal);
}
