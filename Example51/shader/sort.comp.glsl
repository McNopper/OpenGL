#version 460 core

layout(local_size_x = 256) in;

layout(std430, binding = 1) buffer IndexBuffer { uint  indices[]; };
layout(std430, binding = 2) buffer DepthBuffer { float depths[];  };

// Current bitonic merge parameters.
uniform uint u_j;
uniform uint u_k;

void main()
{
    uint i   = gl_GlobalInvocationID.x;
    uint ixj = i ^ u_j;

    // Each pair is handled exactly once: only the thread with the lower index acts.
    if (ixj <= i)
        return;

    float di = depths[i];
    float dj = depths[ixj];

    // Descending sort: larger distances (farther splats) end up at lower indices
    // so they are rendered first (back-to-front order).
    bool ascending = ((i & u_k) == 0u);
    bool doSwap    = ascending ? (di < dj) : (di > dj);

    if (doSwap)
    {
        depths[i]   = dj;
        depths[ixj] = di;

        uint tmp     = indices[i];
        indices[i]   = indices[ixj];
        indices[ixj] = tmp;
    }
}
