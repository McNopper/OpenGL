#version 460 core

/**
 * Odd-even transposition sort — shared memory variant.
 *
 * GRID_N is injected from the host at compile time.
 * Each work group handles one complete line of the 3D texture.
 * GRID_N passes of odd-even compare-swap fully sort the line.
 *
 * Two barriers per pass guarantee correctness across warps:
 *   1. after the read phase  — no thread writes before all reads are done
 *   2. after the write phase — next pass only starts when all writes are visible
 *
 * u_axis: 0 = sort all X-rows    by R
 *         1 = sort all Y-columns by G
 *         2 = sort all Z-tubes   by B
 */

layout(local_size_x = GRID_N) in;

layout(rgba8, binding = 0) uniform image3D u_colorImage;

uniform int u_axis;

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

    // Load line into shared memory.
    s_line[lid] = imageLoad(u_colorImage, pos);
    barrier();

    for (int pass = 0; pass < GRID_N; pass++)
    {
        // Determine pair partner for this pass.
        uint neighbor;
        bool paired;

        if ((pass & 1) == 0)
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

        // Read phase: capture both values before any thread writes.
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
    }

    imageStore(u_colorImage, pos, s_line[lid]);
}
