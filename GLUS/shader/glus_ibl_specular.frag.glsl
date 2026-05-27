/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) since 2010 Norbert Nopper
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#version 410 core

// Pre-filter the environment map for a given roughness level using GGX importance
// sampling with the N=V=R split-sum approximation (Karis / Epic 2013).
// Samples from a mipmapped cubemap (not directly from the equirectangular panorama)
// to avoid distortion artefacts near the poles.
//
// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// see https://github.com/KhronosGroup/glTF-Sample-Renderer

#define GLUS_PI 3.1415926535897932384626433832795
#define NUM_SAMPLES 1024

uniform samplerCube u_cubemapTexture;
uniform int         u_face;
uniform float       u_roughness;
uniform int         u_width;

in vec2  v_texCoord;
out vec4 fragColor;

// Same face mapping as glus_ibl_background.frag.glsl (OpenGL Table 8.19).
vec3 cubemapDirection(int face, vec2 uv)
{
    switch (face)
    {
    case 0: return normalize(vec3(1.0, -uv.y, -uv.x));  // +X
    case 1: return normalize(vec3(-1.0, -uv.y, uv.x));  // -X
    case 2: return normalize(vec3(uv.x, 1.0, uv.y));    // +Y
    case 3: return normalize(vec3(uv.x, -1.0, -uv.y));  // -Y
    case 4: return normalize(vec3(uv.x, -uv.y, 1.0));   // +Z
    case 5: return normalize(vec3(-uv.x, -uv.y, -1.0)); // -Z
    }
    return vec3(0.0);
}

// Hammersley low-discrepancy sequence (Van der Corput radical inverse).
// see http://mathworld.wolfram.com/HammersleyPointSet.html
float radicalInverse_VdC(uint bits)
{
    bits = (bits << 16u) | (bits >> 16u);
    bits = ((bits & 0x00FF00FFu) << 8u) | ((bits & 0xFF00FF00u) >> 8u);
    bits = ((bits & 0x0F0F0F0Fu) << 4u) | ((bits & 0xF0F0F0F0u) >> 4u);
    bits = ((bits & 0x33333333u) << 2u) | ((bits & 0xCCCCCCCCu) >> 2u);
    bits = ((bits & 0x55555555u) << 1u) | ((bits & 0xAAAAAAAAu) >> 1u);
    return float(bits) * 2.3283064365386963e-10;
}

vec2 hammersley(uint i, uint N)
{
    return vec2(float(i) / float(N), radicalInverse_VdC(i));
}

// GGX microfacet importance sampling — returns H in tangent space (z = up).
// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
vec3 importanceSampleGGX(vec2 xi, float roughness)
{
    float alpha    = roughness * roughness;
    float phi      = 2.0 * GLUS_PI * xi.x;
    float cosTheta = sqrt((1.0 - xi.y) / (1.0 + (alpha * alpha - 1.0) * xi.y));
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);
    return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

void main(void)
{
    // N = V = R (split-sum N=V approximation).
    vec3 N = cubemapDirection(u_face, v_texCoord);
    vec3 V = N;

    // Build an orthonormal basis around N.
    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3  color       = vec3(0.0);
    float totalWeight = 0.0;

    for (uint i = 0u; i < uint(NUM_SAMPLES); i++)
    {
        vec2 xi    = hammersley(i, uint(NUM_SAMPLES));
        vec3 H_tan = importanceSampleGGX(xi, u_roughness);
        vec3 H     = normalize(tangent * H_tan.x + bitangent * H_tan.y + N * H_tan.z);

        vec3  L     = reflect(-V, H);
        float NdotL = max(dot(N, L), 0.0);

        if (NdotL > 0.0)
        {
            float NdotH = max(dot(N, H), 0.0);
            float HdotV = max(dot(H, V), 0.0);

            // GGX PDF.
            float alpha = u_roughness * u_roughness;
            float D     = alpha * alpha / (float(GLUS_PI) * pow(NdotH * NdotH * (alpha * alpha - 1.0) + 1.0, 2.0));
            float pdf   = D * NdotH / (4.0 * HdotV + 0.0001);
            // Mipmap Filtered Samples (GPU Gems 3, 20.4); bias factor matches glTF Sample Viewer.
            // see https://developer.nvidia.com/gpugems/gpugems3/part-iii-rendering/chapter-20-gpu-based-importance-sampling
            float mipLevel = u_roughness == 0.0 ? 0.0 : max(0.5 * log2(6.0 * float(u_width) * float(u_width) / (float(NUM_SAMPLES) * (pdf + 0.0001))), 0.0);

            color += textureLod(u_cubemapTexture, L, mipLevel).rgb * NdotL;
            totalWeight += NdotL;
        }
    }

    fragColor = vec4(totalWeight > 0.0 ? color / totalWeight : color, 1.0);
}
