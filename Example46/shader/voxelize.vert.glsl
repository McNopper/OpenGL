#version 460 core

// Voxelization vertex shader.
//
// Reference:
// Cyril Crassin, Fabrice Neyret, Miguel Sainz, Simon Green, Elmar Eisemann,
// "Interactive Indirect Illumination Using Voxel Cone Tracing",
// Pacific Graphics 2011.
// https://research.nvidia.com/sites/default/files/pubs/2011-09_Interactive-Indirect-Illumination/GIVoxels-pg2011-authors.pdf

layout(location = 0) in vec4 a_vertex;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_modelMatrix;

out vec3 v_worldPosition;
out vec3 v_normal;
out vec2 v_texCoord;

void main()
{
    vec4 worldPos   = u_modelMatrix * a_vertex;
    v_worldPosition = worldPos.xyz;
    v_normal        = normalize(mat3(u_modelMatrix) * a_normal);
    v_texCoord      = a_texCoord;

    // Pass world position as clip position; the geometry shader remaps it.
    gl_Position = worldPos;
}
