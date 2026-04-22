#version 440 core

// Voxelization fragment shader.
//
// Computes direct Lambertian illumination for each surface fragment and stores
// the resulting radiance into the RGBA16F 3-D voxel texture via imageStore.
// The alpha channel is set to 1 to mark occupied voxels.
//
// Reference:
// Cyril Crassin et al., "Interactive Indirect Illumination Using Voxel Cone
// Tracing", Pacific Graphics 2011.
// https://research.nvidia.com/sites/default/files/pubs/2011-09_Interactive-Indirect-Illumination/GIVoxels-pg2011-authors.pdf

in vec3 g_worldPosition;
in vec3 g_normal;
in vec2 g_texCoord;

layout(rgba16f, binding = 0) uniform writeonly image3D u_voxelGrid;

layout(binding = 1) uniform sampler2D u_diffuseTexture;
uniform vec4 u_diffuseColor;
uniform int  u_hasDiffuseTexture;

uniform vec3 u_lightPos;
uniform vec3 u_lightColor;

// When non-zero the fragment stores its diffuse colour directly as emissive
// radiance, bypassing the Lambertian lighting calculation.
uniform int  u_isEmissive;

uniform int  u_voxelGridSize;

void main()
{
    // Map world position from [-1, 1]^3 to voxel integer coordinates [0, N).
    vec3  texPos   = g_worldPosition * 0.5 + 0.5;
    ivec3 voxelPos = ivec3(texPos * float(u_voxelGridSize));

    if (any(lessThan(voxelPos, ivec3(0))) ||
        any(greaterThanEqual(voxelPos, ivec3(u_voxelGridSize))))
        return;

    // Resolve material diffuse colour.
    vec3 albedo;
    float alpha;
    if (u_hasDiffuseTexture != 0)
    {
        vec4 tex = texture(u_diffuseTexture, g_texCoord);
        albedo   = tex.rgb;
        alpha    = tex.a;
    }
    else
    {
        albedo = u_diffuseColor.rgb;
        alpha  = u_diffuseColor.a;
    }

    // Discard fully transparent fragments (e.g. leaves mask).
    if (alpha < 0.1)
        return;

    vec3 radiance;
    if (u_isEmissive != 0)
    {
        // Emissive material: store colour at 8x brightness so the value
        // survives mip averaging and remains visible to distant cone traces.
        radiance = albedo * 8.0;
    }
    else
    {
        // Direct Lambertian illumination from the single point light.
        vec3  N     = normalize(g_normal);
        vec3  L     = normalize(u_lightPos - g_worldPosition);
        float NdotL = max(dot(N, L), 0.0);

        // Small ambient term prevents completely dark voxels.
        radiance = (u_lightColor * NdotL + 0.15) * albedo;
    }

    imageStore(u_voxelGrid, voxelPos, vec4(radiance, 1.0));
}
