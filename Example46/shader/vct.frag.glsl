#version 440 core

// Voxel Cone Tracing fragment shader.
//
// Implements real-time global illumination by tracing cones through a
// pre-voxelised radiance volume:
//
//  - Indirect diffuse:  6 cones covering the upper hemisphere (60° aperture).
//  - Indirect specular: 1 narrow cone along the reflection vector.
//  - Direct light:      Blinn-Phong with the point light.
//  - Ambient occlusion: derived from the diffuse cone alpha accumulation.
//
// Reference:
// Cyril Crassin, Fabrice Neyret, Miguel Sainz, Simon Green, Elmar Eisemann,
// "Interactive Indirect Illumination Using Voxel Cone Tracing",
// Pacific Graphics 2011.
// https://research.nvidia.com/sites/default/files/pubs/2011-09_Interactive-Indirect-Illumination/GIVoxels-pg2011-authors.pdf

in vec3 v_worldPosition;
in vec3 v_normal;
in vec2 v_texCoord;

out vec4 fragColor;

// Voxel radiance volume (mip-mapped RGBA8).
layout(binding = 0) uniform sampler3D u_voxelGrid;

// Per-material diffuse texture.
layout(binding = 1) uniform sampler2D u_diffuseTexture;
uniform vec4  u_diffuseColor;
uniform vec4  u_specularColor;
uniform float u_shininess;
uniform int   u_hasDiffuseTexture;

// Lighting.
uniform vec3 u_lightPos;
uniform vec3 u_lightColor;

// Camera.
uniform vec3 u_cameraPos;

// Voxel grid parameters.
uniform float u_voxelGridWorldSize;  // World-space extent of the grid (2.0 for [-1,1]^3).
uniform int   u_voxelDimensions;     // Number of voxels per axis.

// Front-to-back alpha at which cone marching terminates early.
const float ALPHA_THRESH = 0.95;

// -----------------------------------------------------------------------
// 6-cone hemisphere sampling configuration (Crassin et al. 2011, Fig. 7).
//
// One cone aligned with the surface normal and five cones tilted 60 degrees
// from it, equally spaced around the azimuth.  All cones use a 60-degree
// full aperture (tan(30°) ≈ 0.5774 as the tanHalfAngle argument).
// The solid-angle weights sum to 1.0.
// -----------------------------------------------------------------------
const int NUM_CONES = 6;

vec3 coneDirections[6] = vec3[](
    vec3( 0.000000,  1.000000,  0.000000),
    vec3( 0.000000,  0.500000,  0.866025),
    vec3( 0.823639,  0.500000,  0.267617),
    vec3( 0.509037,  0.500000, -0.700629),
    vec3(-0.509037,  0.500000, -0.700629),
    vec3(-0.823639,  0.500000,  0.267617)
);

float coneWeights[6] = float[](0.25, 0.15, 0.15, 0.15, 0.15, 0.15);

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

// Fetch a trilinear sample from the voxel grid at a world-space position.
// worldPos is expected in the same space as the voxel grid: [-1, 1]^3.
// Returns vec4(0) for positions outside the voxel volume to prevent border
// texel leakage on CLAMP_TO_EDGE wrapped textures.
vec4 sampleVoxels(vec3 worldPos, float lod)
{
    // Map [-1, 1]^3 -> [0, 1]^3.  u_voxelGridWorldSize == 2.0, so
    // dividing by (worldSize * 0.5) == 1.0 is a no-op but makes the
    // formula explicit and correct for other world sizes.
    vec3 uv = worldPos / (u_voxelGridWorldSize * 0.5) * 0.5 + 0.5;

    // Terminate outside the volume so border texels do not bleed.
    if (any(lessThan(uv, vec3(0.0))) || any(greaterThan(uv, vec3(1.0))))
        return vec4(0.0);

    return textureLod(u_voxelGrid, uv, lod);
}

// Trace a single cone through the voxel radiance volume using front-to-back
// alpha compositing.
//
// direction:    world-space cone axis (normalised).
// tanHalfAngle: tangent of the cone half-angle (controls spread rate).
// occlusion:    output — accumulated occlusion along the cone.
//
// Returns accumulated colour in rgb and alpha in a.
vec4 coneTrace(vec3 direction, float tanHalfAngle, out float occlusion)
{
    float voxelSize = u_voxelGridWorldSize / float(u_voxelDimensions);

    // Start one voxel away from the surface along the interpolated normal to
    // avoid self-occlusion on flat surfaces.  Begin marching from dist = 0 so
    // that the first sample lands exactly at startPos (no extra skip).
    vec3  startPos = v_worldPosition + normalize(v_normal) * voxelSize;
    float dist     = 0.0;
    float maxDist  = u_voxelGridWorldSize;

    vec3  color   = vec3(0.0);
    float alpha   = 0.0;
    occlusion     = 0.0;

    while (dist < maxDist && alpha < ALPHA_THRESH)
    {
        // The cone diameter grows with distance.
        float diameter = max(voxelSize, 2.0 * tanHalfAngle * dist);
        float lod      = log2(diameter / voxelSize);

        vec4 voxelColor = sampleVoxels(startPos + dist * direction, lod);

        // Front-to-back compositing (Porter-Duff over operator).
        float a  = 1.0 - alpha;
        color   += a * voxelColor.rgb;
        alpha   += a * voxelColor.a;

        // Occlusion attenuated by cone diameter so distant blockers contribute
        // less (reduces over-darkening of large open areas).
        occlusion += (a * voxelColor.a) / (1.0 + 0.03 * diameter);

        // Advance by half the diameter for smooth, overlapping samples.
        dist += diameter * 0.5;
    }

    return vec4(color, alpha);
}

// Trace six diffuse cones over the hemisphere defined by the surface normal,
// returning total indirect irradiance and the derived ambient occlusion.
vec4 indirectLight(out float occlusion_out)
{
    vec4 color  = vec4(0.0);
    occlusion_out = 0.0;

    // Build an orthonormal tangent frame with the Y axis along the surface
    // normal so the cone directions (defined in tangent space) map to world
    // space correctly.
    vec3 N = normalize(v_normal);
    vec3 T = normalize(cross(abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0)
                                               : vec3(1.0, 0.0, 0.0), N));
    vec3 B = cross(N, T);
    mat3 tangentToWorld = mat3(T, N, B);

    for (int i = 0; i < NUM_CONES; i++)
    {
        float occlusion = 0.0;
        // 60-degree aperture per cone → tanHalfAngle = tan(30°) ≈ 0.5774.
        color         += coneWeights[i] * coneTrace(tangentToWorld * coneDirections[i], 0.5774, occlusion);
        occlusion_out += coneWeights[i] * occlusion;
    }

    // Convert from raw occlusion to an accessibility factor in [0, 1].
    occlusion_out = 1.0 - occlusion_out;

    return color;
}

// -----------------------------------------------------------------------
// Main
// -----------------------------------------------------------------------
void main()
{
    // Resolve material colour.
    vec3  albedo;
    float alpha;
    if (u_hasDiffuseTexture != 0)
    {
        vec4 tex = texture(u_diffuseTexture, v_texCoord);
        albedo   = tex.rgb;
        alpha    = tex.a;
    }
    else
    {
        albedo = u_diffuseColor.rgb;
        alpha  = u_diffuseColor.a;
    }

    if (alpha < 0.1)
        discard;

    vec3 N = normalize(v_normal);
    vec3 L = normalize(u_lightPos - v_worldPosition);
    vec3 V = normalize(u_cameraPos - v_worldPosition);
    vec3 H = normalize(L + V);

    // Direct diffuse (Lambertian).
    float NdotL       = max(dot(N, L), 0.0);
    vec3  directDiff  = u_lightColor * NdotL;

    // Direct specular (Blinn-Phong).
    float NdotH       = max(dot(N, H), 0.0);
    vec3  directSpec  = u_lightColor * pow(NdotH, max(u_shininess, 1.0)) * u_specularColor.rgb;

    // Indirect diffuse via hemisphere cone tracing.
    float occlusion   = 0.0;
    vec3  indirDiff   = 4.0 * indirectLight(occlusion).rgb;

    // Ambient occlusion derived from cone alpha accumulation.
    occlusion = min(1.0, 1.5 * occlusion);

    // Indirect specular via a narrow cone along the reflection vector.
    // Cone aperture is derived from material shininess using the Phong lobe
    // solid-angle relationship: tan(halfAngle) = sqrt(2 / (shininess + 2)).
    // Clamped to [0.02, 0.577] — nearly mirror to 60-degree diffuse spread.
    vec3  R                = reflect(-V, N);
    float specTanHalfAngle = clamp(sqrt(2.0 / max(u_shininess + 2.0, 1.0)), 0.02, 0.5774);
    float specOcc;
    vec3  indirSpec        = 2.0 * u_specularColor.rgb * coneTrace(R, specTanHalfAngle, specOcc).rgb;

    // Combine direct and indirect contributions modulated by AO.
    vec3 finalColor =
        albedo   * occlusion * (directDiff + indirDiff) +
        occlusion * (directSpec + indirSpec);

    fragColor = vec4(finalColor, alpha);
}
