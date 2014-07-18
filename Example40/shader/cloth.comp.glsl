#version 430 core

// Number of iterations the spring constraints and collisions are relaxed.
// The more relaxations, the more calm, but also slower is the simulation. 
#define RELAXATIONS 4

// Influences the spring relaxation vector. Has to be:
// 0.0 < STIFFNESS < 1.0
#define STIFFNESS 0.7

// Simulating friction. The less friction, the faster the cloth drops down the sphere.
#define FRICTION 1.0

// Unit: kg * m * s*s
#define GRAVITY_FORCE vec4(0.0, -0.2, 0.0, 0.0)

// Unit: kg
#define MASS 0.1

// Offset, that a particle is not inside the sphere surface.
#define RADIUS_TOLERANCE 0.01

uniform int u_verticesPerRow;

uniform float u_deltaTime;

// Distance at rest between vertical and horizontal particle.
uniform float u_distanceRest;
// Distance at rest between disagonal particle.
uniform float u_distanceDiagonalRest;

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

vec4 calculateSpringVector(vec4 a, vec4 b, float distanceRest)
{
    vec4 deltaVector = a - b;
    
    float deltaLength = length(deltaVector);
    
    return 0.5 * deltaVector * (distanceRest - deltaLength) / deltaLength * STIFFNESS;
}

vec3 gramSchmidt(vec3 u, vec3 v)
{
    vec3 uProjV;
    
    float vDotU;
    
    float uDotU = dot(u, u);
    
    if (uDotU == 0.0)
    {
        return vec3(0.0, 0.0, 0.0);
    }
    
    vDotU = dot(v, u);
    
    uProjV = u * (vDotU / uDotU);
    
    return v - uProjV;
}

// see http://web.archive.org/web/20070610223835/http://www.teknikus.dk/tj/gdc2001.htm
//     http://developer.download.nvidia.com/SDK/10/direct3d/Source/Cloth/doc/Cloth.pdf
void main(void)
{
    int currentIndex = int(gl_GlobalInvocationID.x);
    
    int currentColumn = currentIndex % u_verticesPerRow;
    int currentRow = currentIndex / u_verticesPerRow;
    
    //
    // Summing and calcualting all forces, to get the acceleration.
    // 
    
    vec4 force = vec4(0.0, 0.0, 0.0, 0.0);
    
    vec4 normalVector = b_vertexInCurrent[currentIndex] - u_sphereCenter;
    
    // Test, if particle is on sphere.
    if (length(normalVector) <= u_sphereRadius + RADIUS_TOLERANCE)
    {
        vec3 tangentVector = vec3(0.0, 0.0, 0.0);
        
        normalVector = normalize(normalVector);

        float cosAlpha = max(dot(normalVector.xyz, vec3(0.0, 1.0, 0.0)), 0.0); 
        
        if (cosAlpha != 1.0)
        {
            tangentVector = normalize(gramSchmidt(normalVector.xyz, vec3(0.0, 1.0, 0.0)));
        }
        
        //
        // Add grade restistance and friction force.
        //
        
        vec4 Fa = length(GRAVITY_FORCE) * sin(acos(cosAlpha)) * vec4(-tangentVector, 0.0);
        vec4 Fr = FRICTION * length(GRAVITY_FORCE) * cosAlpha * vec4(-tangentVector, 0.0);
        
        force += Fa - Fr;
    }
    else
    {
        //
        // Add plain gravity force.
        //
        
        force += GRAVITY_FORCE;
    }
    
    vec4 a = force / MASS;
    
    //
    // Verlet integration.
    //
    
    float squareDeltaTime = u_deltaTime * u_deltaTime;
    
    vec4 x = b_vertexInCurrent[currentIndex];
    
    vec4 xPrev = b_vertexInPrevious[currentIndex];
    
    b_vertexOut[currentIndex] = 2.0 * x - xPrev + a * squareDeltaTime;
    
    memoryBarrierBuffer();
    
    barrier();
    
    //
    // Relaxation iterations.
    //
    for (int relax = 0; relax < RELAXATIONS; relax++)
    {
        //
        // Process spring constraints.
        //
        
        // Alternating "even" and "odd" cells to avoid dependencies.
        for (int evenOdd = 0; evenOdd < 2; evenOdd++)
        {
            int rightIndex;
            int belowIndex;
            int diagonalIndex;
            
            vec4 springVector;
            
            if (currentRow % 2 == evenOdd && currentColumn % 2 == evenOdd)
            {
                vec4 current = b_vertexOut[currentIndex];
                vec4 right;
                vec4 below;
                vec4 diagonal;
                
                rightIndex = currentIndex + 1;
                if (rightIndex % u_verticesPerRow > currentColumn)
                {
                    right = b_vertexOut[rightIndex];
                
                    springVector = calculateSpringVector(current, right, u_distanceRest);
                    
                    b_vertexOut[currentIndex] += springVector;
                    b_vertexOut[rightIndex] -= springVector;
                    
                    memoryBarrierBuffer();
                }
                
                
                belowIndex = currentIndex + u_verticesPerRow;
                if (belowIndex < u_verticesPerRow * u_verticesPerRow)
                {
                    below = b_vertexOut[belowIndex];
                    
                    springVector = calculateSpringVector(current, below, u_distanceRest);
                    
                    b_vertexOut[currentIndex] += springVector;
                    b_vertexOut[belowIndex] -= springVector;
                    
                    memoryBarrierBuffer();  
                }
                
                
                diagonalIndex = currentIndex + 1 + u_verticesPerRow;
                if (diagonalIndex < u_verticesPerRow * u_verticesPerRow && (diagonalIndex % u_verticesPerRow) > currentColumn)
                {
                    diagonal = b_vertexOut[diagonalIndex];
                    
                    springVector = calculateSpringVector(right, diagonal, u_distanceRest);
                    
                    b_vertexOut[rightIndex] += springVector;
                    b_vertexOut[diagonalIndex] -= springVector;
                    
                    memoryBarrierBuffer();
                    
                    springVector = calculateSpringVector(below, diagonal, u_distanceRest);
                    
                    b_vertexOut[belowIndex] += springVector;
                    b_vertexOut[diagonalIndex] -= springVector;
                    
                    memoryBarrierBuffer();
                    
                    //
                    // Diagonal
                    //
                    
                    springVector = calculateSpringVector(current, diagonal, u_distanceDiagonalRest);
                    
                    b_vertexOut[currentIndex] += springVector;
                    b_vertexOut[diagonalIndex] -= springVector;

					// No memory barrier by purpose
                    
                    springVector = calculateSpringVector(right, below, u_distanceDiagonalRest);
                    
                    b_vertexOut[rightIndex] += springVector;
                    b_vertexOut[belowIndex] -= springVector;
                    
                    memoryBarrierBuffer();
                }
            }
            else if (evenOdd == 1 && currentColumn % 2 == 1 && currentRow == 0)
            {
                // First row springs, when processing odd pass.
                
                rightIndex = currentIndex + 1;
                if (rightIndex < u_verticesPerRow)
                {
                    springVector = calculateSpringVector(b_vertexOut[currentIndex], b_vertexOut[rightIndex], u_distanceRest);
                    
                    b_vertexOut[currentIndex] += springVector;
                    b_vertexOut[rightIndex] -= springVector;
                    
                    memoryBarrierBuffer();
                }
            }
            else if (evenOdd == 1 && currentRow % 2 == 1 && currentColumn == 0)
            {
                // First column springs, when processing odd pass.
                
                belowIndex = currentIndex + u_verticesPerRow;
                if (belowIndex < u_verticesPerRow * u_verticesPerRow)
                {
                    springVector = calculateSpringVector(b_vertexOut[currentIndex], b_vertexOut[belowIndex], u_distanceRest);
                    
                    b_vertexOut[currentIndex] += springVector;
                    b_vertexOut[belowIndex] -= springVector;
                    
                    memoryBarrierBuffer();
                }
            }
            
            barrier();
        }
        
        //
        // Separate diagonal iterations.
        //
        
        // Alternating "even" and "odd" cells to avoid dependencies.
        for (int evenOdd = 0; evenOdd < 2; evenOdd++)
        {
            int rightIndex;
            int belowIndex;
            int diagonalIndex;
            
            vec4 springVector;
            
            if (currentRow % 2 == evenOdd && (currentColumn + 1) % 2 == evenOdd)
            {
                vec4 current = b_vertexOut[currentIndex];
                vec4 right;
                vec4 below;
                vec4 diagonal;
                
                rightIndex = currentIndex + 1;
                belowIndex = currentIndex + u_verticesPerRow;
                diagonalIndex = currentIndex + 1 + u_verticesPerRow;
                if (diagonalIndex < u_verticesPerRow * u_verticesPerRow && (diagonalIndex % u_verticesPerRow) > currentColumn)
                {
                    right = b_vertexOut[rightIndex];
                    below = b_vertexOut[belowIndex];
                    diagonal = b_vertexOut[diagonalIndex];
                    
                    //
                    // Diagonal
                    //
                    springVector = calculateSpringVector(current, diagonal, u_distanceDiagonalRest);
                    
                    b_vertexOut[currentIndex] += springVector;
                    b_vertexOut[diagonalIndex] -= springVector;
                    
                    // No memory barrier by purpose
                    
                    springVector = calculateSpringVector(right, below, u_distanceDiagonalRest);
                    
                    b_vertexOut[rightIndex] += springVector;
                    b_vertexOut[belowIndex] -= springVector;
                    
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
            // ... move back on top of sphere.
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
    if (currentColumn < u_verticesPerRow - 1)
    {
        tangent = normalize((b_vertexOut[currentIndex + 1] - b_vertexOut[currentIndex]).xyz);
        
        if (currentRow < u_verticesPerRow - 1)
        {
            bitangent = normalize((b_vertexOut[currentIndex + u_verticesPerRow] - b_vertexOut[currentIndex]).xyz);
            
            normal += normalize(cross(bitangent, tangent));
        }
        if (currentRow > 0)
        {
            bitangent = normalize((b_vertexOut[currentIndex] - b_vertexOut[currentIndex - u_verticesPerRow]).xyz);
            
            normal += normalize(cross(bitangent, tangent));
        }
    }
    if (currentColumn > 0)
    {
        tangent = normalize((b_vertexOut[currentIndex] - b_vertexOut[currentIndex - 1]).xyz);
        
        if (currentRow < u_verticesPerRow - 1)
        {
            bitangent = normalize((b_vertexOut[currentIndex + u_verticesPerRow] - b_vertexOut[currentIndex]).xyz);
            
            normal += normalize(cross(bitangent, tangent));
        }
        if (currentRow > 0)
        {
            bitangent = normalize((b_vertexOut[currentIndex] - b_vertexOut[currentIndex - u_verticesPerRow]).xyz);
            
            normal += normalize(cross(bitangent, tangent));
        }
    }
    
    //
    
    b_normalOut[currentIndex] = normalize(normal);
    
    memoryBarrierBuffer();
}
