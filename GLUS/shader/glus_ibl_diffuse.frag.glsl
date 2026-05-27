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

// Convolve the environment map with a cosine lobe to produce a diffuse irradiance
// cubemap.  Uses cosine-weighted hemisphere sampling; because PDF = NdotL/PI and
// BRDF_Lambert = albedo/PI, both factors cancel and the estimator reduces to simply
// summing the incoming radiance and dividing by the sample count.
// Samples from a mipmapped cubemap (not directly from the equirectangular panorama)
// to avoid distortion artefacts near the poles.
//
// see http://www.pbr-book.org/ Chapter 13 (Monte Carlo Integration)
// see https://github.com/KhronosGroup/glTF-Sample-Renderer

#define GLUS_PI 3.1415926535897932384626433832795
#define NUM_SAMPLES 512

uniform samplerCube u_cubemapTexture;
uniform int         u_face;
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

// Cosine-weighted hemisphere sample (z = up in tangent space).
// see Physically Based Rendering Chapter 13.6.1
vec3 cosineWeightedSample(vec2 xi)
{
    float phi      = 2.0 * GLUS_PI * xi.x;
    float cosTheta = sqrt(xi.y);
    float sinTheta = sqrt(1.0 - xi.y);
    return vec3(sinTheta * cos(phi), sinTheta * sin(phi), cosTheta);
}

void main(void)
{
    vec3 N = cubemapDirection(u_face, v_texCoord);

    vec3 up        = abs(N.z) < 0.999 ? vec3(0.0, 0.0, 1.0) : vec3(1.0, 0.0, 0.0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    vec3 irradiance = vec3(0.0);

    for (uint i = 0u; i < uint(NUM_SAMPLES); i++)
    {
        vec2 xi    = hammersley(i, uint(NUM_SAMPLES));
        vec3 L_tan = cosineWeightedSample(xi);
        vec3 L     = normalize(tangent * L_tan.x + bitangent * L_tan.y + N * L_tan.z);

        // Cosine-weighted PDF = cosTheta / PI; bias factor matches glTF Sample Viewer.
        float cosTheta = max(L_tan.z, 0.0001);
        float pdf      = cosTheta / float(GLUS_PI);
        float mipLevel = max(0.5 * log2(6.0 * float(u_width) * float(u_width) / (float(NUM_SAMPLES) * (pdf + 0.0001))), 0.0);

        irradiance += textureLod(u_cubemapTexture, L, mipLevel).rgb;
    }

    fragColor = vec4(irradiance / float(NUM_SAMPLES), 1.0);
}
