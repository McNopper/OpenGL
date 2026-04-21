#version 440 core

// Dominant-axis projection + conservative rasterization geometry shader.
//
// 1. Projects each triangle onto the axis-aligned plane that maximises its
//    projected area (dominant normal component), so that the rasterizer
//    generates a fragment for every voxel the triangle passes through.
// 2. Expands the projected triangle outward by half a voxel using the
//    edge-plane technique so the rasterizer covers every voxel the original
//    triangle touches — conservative rasterization.
//
// References:
// Cyril Crassin et al., "Interactive Indirect Illumination Using Voxel Cone
// Tracing", Pacific Graphics 2011.
// M. Hasselgren et al., "Conservative and Tiled Rasterization Using a Modified
// Triangle Setup", Journal of Graphics Tools, 2005.
// GPU Gems 2, Chapter 42: Conservative Rasterization.
// http://http.developer.nvidia.com/GPUGems2/gpugems2_chapter42.html

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 v_worldPosition[];
in vec3 v_normal[];
in vec2 v_texCoord[];

out vec3 g_worldPosition;
out vec3 g_normal;
out vec2 g_texCoord;

// Half the size of one voxel in NDC units: 1.0 / VCT_GRID_SIZE.
uniform vec2 u_halfPixelSize;

void main()
{
    // Compute the un-normalised face normal to identify the dominant axis.
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;
    vec3 faceNormal = abs(cross(p1 - p0, p2 - p0));

    // Project all three vertices onto the dominant-axis plane.
    // World space is normalised to [-1, 1]^3 so the 2D result is already in NDC.
    vec2 pos[3];
    for (int i = 0; i < 3; i++)
    {
        vec3 wp = v_worldPosition[i];
        if (faceNormal.z >= faceNormal.x && faceNormal.z >= faceNormal.y)
            pos[i] = wp.xy;   // project onto XY plane
        else if (faceNormal.x >= faceNormal.y && faceNormal.x >= faceNormal.z)
            pos[i] = wp.yz;   // project onto YZ plane
        else
            pos[i] = wp.xz;   // project onto XZ plane
    }

    // Conservative expansion via edge-plane intersection (GPU Gems 2, Ch.42).
    //
    // Each edge defines a homogeneous 2D plane through the origin:
    //   plane[i] = cross( vec3(pos[i], 1), vec3(pos[(i+2)%3], 1) )
    //
    // The plane is pushed outward by half a voxel so the rasterised triangle
    // covers all voxels the original triangle touches.
    vec3 plane[3];
    for (int i = 0; i < 3; i++)
    {
        plane[i]   = cross(vec3(pos[i], 1.0), vec3(pos[(i + 2) % 3], 1.0));
        plane[i].z -= dot(u_halfPixelSize, abs(plane[i].xy));
    }

    // New vertices are the pairwise intersections of the expanded edge planes.
    for (int i = 0; i < 3; i++)
    {
        vec3 intersect = cross(plane[i], plane[(i + 1) % 3]);

        // Skip degenerate (zero-area) triangles.
        if (abs(intersect.z) < 1e-6)
            return;

        g_worldPosition = v_worldPosition[i];
        g_normal        = v_normal[i];
        g_texCoord      = v_texCoord[i];

        gl_Position = vec4(intersect.xy / intersect.z, 0.0, 1.0);
        EmitVertex();
    }

    EndPrimitive();
}
