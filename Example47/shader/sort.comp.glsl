#version 460 core

/**
 * Odd-even transposition sort — OpenGL 4.6 subgroup-shuffle variant.
 *
 * GRID_N is injected from the host after querying GL_SUBGROUP_SIZE_KHR so
 * that local_size_x == gl_SubgroupSize.  With exactly one subgroup per work
 * group every compare-swap uses GL_KHR_shader_subgroup_shuffle — no shared
 * memory, no barriers.
 *
 * gl_SubgroupInvocationID is used as the line index so that shuffle lane
 * addresses always match the actual subgroup lane, independent of how the
 * implementation maps local invocations to subgroup lanes.
 *
 *   Even pass (pass & 1 == 0):  compare-swap pairs (0,1), (2,3), (4,5), ...
 *   Odd  pass (pass & 1 == 1):  compare-swap pairs (1,2), (3,4), (5,6), ...
 *
 * GRID_N passes guarantee full convergence of each line.
 * Repeating axis dispatches 0 → 1 → 2  GRID_N times converges to the
 * RGB cube:  texel(x,y,z).rgb = (x,y,z) * 255 / (GRID_N-1).
 *
 * u_axis:  0 = sort all X-rows    by R
 *          1 = sort all Y-columns by G
 *          2 = sort all Z-tubes   by B
 */

#extension GL_KHR_shader_subgroup_basic   : require
#extension GL_KHR_shader_subgroup_shuffle : require

// GRID_N injected from host (equals gl_SubgroupSize on this device).
layout(local_size_x = GRID_N) in;

layout(rgba8, binding = 0) uniform image3D u_colorImage;

uniform int u_axis;

void main()
{
    // Use gl_SubgroupInvocationID as the line index so shuffle lane addresses
    // always match the actual subgroup lane regardless of invocation ordering.
    uint lid = gl_SubgroupInvocationID;
    int  j   = int(gl_WorkGroupID.x);   // first  transverse coordinate
    int  k   = int(gl_WorkGroupID.y);   // second transverse coordinate

    ivec3 pos;
    if      (u_axis == 0) pos = ivec3(int(lid), j, k);
    else if (u_axis == 1) pos = ivec3(j, int(lid), k);
    else                  pos = ivec3(j, k, int(lid));

    vec4 myVal = imageLoad(u_colorImage, pos);

    for (int pass = 0; pass < GRID_N; pass++)
    {
        float myKey = (u_axis == 0) ? myVal.r
                    : (u_axis == 1) ? myVal.g
                    :                 myVal.b;

        vec4  pVal;
        float pKey;

        if ((pass & 1) == 0)
        {
            // Even pass: exchange between lid and lid^1 (0<->1, 2<->3, ...).
            pVal = subgroupShuffleXor(myVal, 1u);
            pKey = (u_axis == 0) ? pVal.r : (u_axis == 1) ? pVal.g : pVal.b;
        }
        else
        {
            // Odd pass: exchange across the (1,2),(3,4),... boundary.
            // Boundary threads (0, GRID_N-1) read themselves — no swap follows.
            uint srcLane;
            if      (lid == 0u)                srcLane = 0u;
            else if (lid == uint(GRID_N) - 1u) srcLane = uint(GRID_N) - 1u;
            else if ((lid & 1u) == 1u)         srcLane = lid + 1u;
            else                               srcLane = lid - 1u;

            pVal = subgroupShuffle(myVal, srcLane);
            pKey = (u_axis == 0) ? pVal.r : (u_axis == 1) ? pVal.g : pVal.b;
        }

        // Lower-in-pair thread wants min; upper wants max.
        // lowerInPair: even lids on even passes, odd lids on odd passes.
        bool lowerInPair = (lid & 1u) == (uint(pass) & 1u);
        bool active      = (pass & 1) == 0
                         || (lid > 0u && lid < uint(GRID_N) - 1u);

        if (active && (lowerInPair ? (myKey > pKey) : (myKey < pKey)))
            myVal = pVal;
    }

    imageStore(u_colorImage, pos, myVal);
}
