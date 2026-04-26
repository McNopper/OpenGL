#version 460 core

/**
 * Point-cloud vertex shader.
 *
 * gl_VertexID is decoded into a 3-D grid position (x,y,z).  The colour is
 * fetched directly from the 3D texture at that integer texel so we always
 * see what is stored in the data structure without any interpolation.
 */

uniform mat4      u_mvpMatrix;
uniform uint      u_gridN;
uniform float     u_pointSize;
uniform sampler3D u_colorTexture;

out vec4 v_color;

void main()
{
    uint idx = uint(gl_VertexID);
    uint N   = u_gridN;

    uint x = idx % N;
    uint y = (idx / N) % N;
    uint z = idx / (N * N);

    // Map integer grid position to [-1, 1]^3.
    vec3 pos = (vec3(x, y, z) / float(N - 1u)) * 2.0 - 1.0;

    gl_Position  = u_mvpMatrix * vec4(pos, 1.0);
    gl_PointSize = u_pointSize;

    // Exact texel fetch - no interpolation.
    v_color = texelFetch(u_colorTexture, ivec3(x, y, z), 0);
}
