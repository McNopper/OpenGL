#version 460 core

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

// -----------------------------------------------------------------------
// Tuning constants
// -----------------------------------------------------------------------

// Alpha threshold below which a fragment is considered fully transparent.
// Lower = keep more semi-transparent geometry (e.g. leaves); higher = stricter.
const float ALPHA_DISCARD_THRESH = 0.1;

// Ambient radiance added to all non-emissive voxels.
// Prevents completely black surfaces when direct light is zero.
// Higher values lift the global brightness floor.
const float VOXEL_AMBIENT = 0.15;

// Emissive brightness multiplier stored into RGBA16F voxels (values > 1 allowed).
// Higher values keep emissive radiance visible in coarser mip levels,
// extending the indirect-light influence radius across the scene.
const float EMISSIVE_BOOST = 8.0;

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
uniform int u_isEmissive;

uniform int u_voxelGridSize;

void main()
{
    // Map world position from [-1, 1]^3 to voxel integer coordinates [0, N).
    vec3  texPos   = g_worldPosition * 0.5 + 0.5;
    ivec3 voxelPos = ivec3(texPos * float(u_voxelGridSize));

    if (any(lessThan(voxelPos, ivec3(0))) ||
        any(greaterThanEqual(voxelPos, ivec3(u_voxelGridSize))))
        return;

    // Resolve material diffuse colour.
    vec3  albedo;
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
    if (alpha < ALPHA_DISCARD_THRESH)
        return;

    vec3 radiance;
    if (u_isEmissive != 0)
    {
        // Emissive material: store colour at 8x brightness so the value
        // survives mip averaging and remains visible to distant cone traces.
        radiance = albedo * EMISSIVE_BOOST;
    }
    else
    {
        // Direct Lambertian illumination from the single point light.
        vec3  N     = normalize(g_normal);
        vec3  L     = normalize(u_lightPos - g_worldPosition);
        float NdotL = max(dot(N, L), 0.0);

        // Small ambient term prevents completely dark voxels.
        radiance = (u_lightColor * NdotL + VOXEL_AMBIENT) * albedo;
    }

    imageStore(u_voxelGrid, voxelPos, vec4(radiance, 1.0));
}
