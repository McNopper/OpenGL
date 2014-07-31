#version 430 core

// see same define in main.c
#define N 128

layout(std430, binding=1) buffer VertexIn {
    vec4 b_vertexIn[];
};

layout(std430, binding=2) buffer NormalOut {
    vec3 b_normalOut[];
    // Padding[]
};

layout (local_size_x = 1) in;

void main(void)
{
	int currentIndex = int(gl_GlobalInvocationID.x) + int(gl_GlobalInvocationID.y) * N;

    //
    // Calculate normal.
    //
        
    b_normalOut[currentIndex] = vec3(0.0, 1.0, 0.0);
}
