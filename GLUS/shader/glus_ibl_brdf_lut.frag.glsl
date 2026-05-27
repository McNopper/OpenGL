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

// Pre-integrate the split-sum BRDF term (Karis / Epic 2013).
//
// For each (NdotV, roughness) pair this shader integrates, over GGX-sampled half-
// vectors H, the contribution of the Fresnel term with F0 factored out:
//
//   integral  F(VdotH) * G(NdotV,NdotL) * VdotH / (NdotH * NdotV)  dH
//     = F0 * scale + bias
//
// where scale = integral( (1-(1-VdotH)^5) * G_Vis ) and
//       bias  = integral( (1-VdotH)^5     * G_Vis )
//
// Result is stored as (scale, bias) in a GL_RG32F texture.
//
// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// see https://bruop.github.io/ibl/#single_scattering_results

#define GLUS_PI 3.1415926535897932384626433832795
#define NUM_SAMPLES 512

in vec2  v_texCoord;
out vec2 fragColor;

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

// Smith-Schlick-GGX geometric shadowing for IBL.
// k = roughness^2 / 2 (IBL variant, not the direct-lighting variant).
// see http://graphicrants.blogspot.com/2013/08/specular-brdf-reference.html
float G_SchlickGGX_IBL(float NdotX, float roughness)
{
    float k = roughness * roughness / 2.0;
    return NdotX / (NdotX * (1.0 - k) + k);
}

float G_Smith(float NdotV, float NdotL, float roughness)
{
    return G_SchlickGGX_IBL(NdotV, roughness) * G_SchlickGGX_IBL(NdotL, roughness);
}

void main(void)
{
    // v_texCoord is in [-1, 1]; remap to [0, 1] for (NdotV, roughness).
    vec2  uv        = v_texCoord * 0.5 + 0.5;
    float NdotV     = max(uv.x, 0.001); // avoid division by zero at grazing
    float roughness = uv.y;

    // In tangent space N = (0,0,1); choose V so dot(N,V) = NdotV.
    vec3 V = vec3(sqrt(1.0 - NdotV * NdotV), 0.0, NdotV);

    float scale = 0.0;
    float bias  = 0.0;

    for (uint i = 0u; i < uint(NUM_SAMPLES); i++)
    {
        vec2 xi = hammersley(i, uint(NUM_SAMPLES));
        vec3 H  = importanceSampleGGX(xi, roughness);
        vec3 L  = reflect(-V, H); // = 2*dot(V,H)*H - V

        float NdotL = max(L.z, 0.0);
        float NdotH = max(H.z, 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NdotL > 0.0)
        {
            float G     = G_Smith(NdotV, NdotL, roughness);
            float G_Vis = G * VdotH / (NdotH * NdotV);
            float Fc    = pow(1.0 - VdotH, 5.0);

            scale += (1.0 - Fc) * G_Vis; // F0 coefficient
            bias += Fc * G_Vis;          // constant bias
        }
    }

    fragColor = vec2(scale, bias) / float(NUM_SAMPLES);
}
