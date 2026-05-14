#version 460 core

#ifndef SPLAT_STRIDE
#define SPLAT_STRIDE 59
#endif

layout(local_size_x = 256) in;

layout(std140, binding = 0) uniform WorldData {
    mat4 projMatrix;
    mat4 viewMatrix;
    vec2 focal;
    vec2 viewport;
    vec4 camPos;
} world;

layout(std430, binding = 0) readonly  buffer SplatBuffer { float splats[]; };
layout(std430, binding = 1) writeonly buffer IndexBuffer { uint  indices[]; };
layout(std430, binding = 2) writeonly buffer DepthBuffer { float depths[];  };

uniform mat4  u_worldMatrix;
uniform uint  u_numSplats;
uniform uint  u_numSplatsPadded;

void main()
{
    uint i = gl_GlobalInvocationID.x;

    if (i >= u_numSplatsPadded)
        return;

    if (i >= u_numSplats)
    {
        indices[i] = i;
        depths[i]  = -1.0e10;
        return;
    }

    uint  base = i * uint(SPLAT_STRIDE);
    vec3  pos  = vec3(splats[base], splats[base + 1u], splats[base + 2u]);
    vec3  wpos = (u_worldMatrix * vec4(pos, 1.0)).xyz;

    // Negate view-space z: farther splats get a larger value (back-to-front order).
    float viewZ = (world.viewMatrix * vec4(wpos, 1.0)).z;

    indices[i] = i;
    depths[i]  = -viewZ;
}
