#version 460 core

/**
 * Odd-even transposition sort — one compare-swap pass per dispatch.
 *
 * GRID_N is injected from the host at compile time.
 * Each work group handles one line of the 3D texture.
 * Each dispatch performs exactly one even or odd compare-swap pass.
 * The host drives GRID_N dispatches per axis to fully sort each line.
 *
 * All barrier() calls are at the top level — no control flow warning.
 *
 * u_axis: 0 = sort all X-rows    by R
 *         1 = sort all Y-columns by G
 *         2 = sort all Z-tubes   by B
 * u_pass: even/odd index of this pass (0 = even pairs, 1 = odd pairs, ...)
 */

layout(local_size_x = GRID_N) in;

layout(rgba8, binding = 0) uniform image3D u_colorImage;

uniform int u_axis;
uniform int u_pass;

shared vec4 s_line[GRID_N];

void main()
{
    uint  lid = gl_LocalInvocationID.x;
    int   j   = int(gl_WorkGroupID.x);
    int   k   = int(gl_WorkGroupID.y);

    ivec3 pos;
    if      (u_axis == 0) pos = ivec3(int(lid), j, k);
    else if (u_axis == 1) pos = ivec3(j, int(lid), k);
    else                  pos = ivec3(j, k, int(lid));

    // Load line into shared memory — barrier is at top level, no warning.
    s_line[lid] = imageLoad(u_colorImage, pos);
    barrier();

    // Determine pair partner for this pass (even or odd).
    uint neighbor;
    bool paired;

    if ((u_pass & 1) == 0)
    {
        // Even pass: pairs (0,1), (2,3), (4,5), ...
        neighbor = lid ^ 1u;
        paired   = true;
    }
    else
    {
        // Odd pass: pairs (1,2), (3,4), (5,6), ...
        // Thread 0 and GRID_N-1 sit at boundaries with no partner.
        if (lid == 0u || lid == uint(GRID_N) - 1u)
        {
            neighbor = lid;
            paired   = false;
        }
        else
        {
            neighbor = ((lid & 1u) == 1u) ? lid + 1u : lid - 1u;
            paired   = true;
        }
    }

    // Read phase — barrier at top level.
    vec4 myVal = s_line[lid];
    vec4 nVal  = s_line[neighbor];
    barrier();

    // Write phase: swap if out of order.
    if (paired)
    {
        float myKey = (u_axis == 0) ? myVal.r : (u_axis == 1) ? myVal.g : myVal.b;
        float nKey  = (u_axis == 0) ? nVal.r  : (u_axis == 1) ? nVal.g  : nVal.b;

        if (lid < neighbor ? (myKey > nKey) : (myKey < nKey))
            s_line[lid] = nVal;
    }
    barrier();

    imageStore(u_colorImage, pos, s_line[lid]);
}
