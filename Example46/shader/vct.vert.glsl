#version 440 core

// VCT rendering vertex shader.
//
// Transforms each vertex to clip space and forwards the world-space position,
// interpolated surface normal and texture coordinate to the fragment shader.
//
// Reference:
// Cyril Crassin et al., "Interactive Indirect Illumination Using Voxel Cone
// Tracing", Pacific Graphics 2011.

layout(location = 0) in vec4 a_vertex;
layout(location = 1) in vec3 a_normal;
layout(location = 2) in vec2 a_texCoord;

uniform mat4 u_modelMatrix;
uniform mat4 u_mvpMatrix;

out vec3 v_worldPosition;
out vec3 v_normal;
out vec2 v_texCoord;

void main()
{
    vec4 worldPos   = u_modelMatrix * a_vertex;
    v_worldPosition = worldPos.xyz;
    v_normal        = normalize(mat3(u_modelMatrix) * a_normal);
    v_texCoord      = a_texCoord;
    gl_Position     = u_mvpMatrix * a_vertex;
}
