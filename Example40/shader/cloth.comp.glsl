#version 430 core

layout(std430, binding=1) buffer VertexIn {
    vec4 b_vertexIn[];
};

layout(std430, binding=2) buffer VertexOut {
    vec4 b_vertexOut[];
};

layout(std430, binding=3) buffer NormalOut {
    vec3 b_normalOut[];
    // Padding[]
};

// If ROWS and COLUMS change in the main file, update local size as well.
// (ROWS+1)*(COLUMNS+1)
layout (local_size_x = 121) in;

void main(void)
{
    uint i = gl_GlobalInvocationID.x;

	// TODO Do cloth simulation, as seen:
	// http://web.archive.org/web/20070610223835/http://www.teknikus.dk/tj/gdc2001.htm
	// http://developer.download.nvidia.com/SDK/10/direct3d/Source/Cloth/doc/Cloth.pdf

    b_vertexOut[i] = b_vertexIn[i];
    b_normalOut[i] = vec3(0.0, 1.0, 0.0);
}
