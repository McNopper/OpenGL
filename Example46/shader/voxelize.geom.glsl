#version 440 core

// Dominant-axis projection geometry shader for scene voxelization.
//
// Projects each triangle onto the axis-aligned plane that maximises its
// projected area (dominant normal component), so that the rasterizer
// generates a fragment for every voxel the triangle passes through.
// World position, interpolated normal and texture coordinate are passed
// unchanged to the fragment shader.
//
// Reference:
// Cyril Crassin et al., "Interactive Indirect Illumination Using Voxel Cone
// Tracing", Pacific Graphics 2011.

layout(triangles) in;
layout(triangle_strip, max_vertices = 3) out;

in vec3 v_worldPosition[];
in vec3 v_normal[];
in vec2 v_texCoord[];

out vec3 g_worldPosition;
out vec3 g_normal;
out vec2 g_texCoord;

void main()
{
    // Compute the un-normalised face normal to identify the dominant axis.
    vec3 p0 = gl_in[0].gl_Position.xyz;
    vec3 p1 = gl_in[1].gl_Position.xyz;
    vec3 p2 = gl_in[2].gl_Position.xyz;
    vec3 faceNormal = abs(cross(p1 - p0, p2 - p0));

    for (int i = 0; i < 3; i++)
    {
        g_worldPosition = v_worldPosition[i];
        g_normal        = v_normal[i];
        g_texCoord      = v_texCoord[i];

        vec3 wp = v_worldPosition[i];

        // Project along the dominant axis so the rasteriser covers the most voxels.
        // World space is normalised to [-1, 1]^3, so the projection coordinates
        // are already in NDC range.
        if (faceNormal.z >= faceNormal.x && faceNormal.z >= faceNormal.y)
            gl_Position = vec4(wp.x, wp.y, 0.0, 1.0);   // project onto XY
        else if (faceNormal.x >= faceNormal.y && faceNormal.x >= faceNormal.z)
            gl_Position = vec4(wp.y, wp.z, 0.0, 1.0);   // project onto YZ
        else
            gl_Position = vec4(wp.x, wp.z, 0.0, 1.0);   // project onto XZ

        EmitVertex();
    }

    EndPrimitive();
}
