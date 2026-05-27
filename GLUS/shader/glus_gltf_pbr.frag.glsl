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

#version 460 core

// PBR metallic-roughness + IBL (split-sum approximation).

uniform sampler2D u_baseColorTexture;
uniform sampler2D u_metallicRoughnessTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_occlusionTexture;
uniform sampler2D u_emissiveTexture;

uniform samplerCubeArray u_specularEnvMap; // pre-filtered specular mip array
uniform samplerCube      u_diffuseEnvMap;  // irradiance cubemap
uniform sampler2D        u_brdfLUT;

// Material factors
uniform vec4  u_baseColorFactor;
uniform float u_metallicFactor;
uniform float u_roughnessFactor;
uniform vec3  u_emissiveFactor;
uniform float u_occlusionStrength;
uniform float u_alphaCutoff;
uniform int   u_alphaMode; // 0=OPAQUE,1=MASK,2=BLEND
uniform int   u_hasNormalMap;

// Number of roughness levels - 1 (cast to float for layer lookup)
uniform float u_roughnessScale;

uniform vec4 u_eye;

in vec3 v_worldPos;
in vec3 v_normal;
in vec4 v_tangent;
in vec2 v_texCoord0;

out vec4 fragColor;

const float PI = 3.14159265358979323846;

// Fresnel-Schlick approximation.
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main(void)
{
    // ---------- Sample textures ----------
    vec4  baseColorSample = texture(u_baseColorTexture, v_texCoord0);
    vec4  mrSample        = texture(u_metallicRoughnessTexture, v_texCoord0);
    vec3  normalSample    = texture(u_normalTexture, v_texCoord0).xyz;
    float aoSample        = texture(u_occlusionTexture, v_texCoord0).r;
    vec3  emissiveSample  = texture(u_emissiveTexture, v_texCoord0).rgb;

    // ---------- Base colour + alpha ----------
    vec4 baseColor = baseColorSample * u_baseColorFactor;

    if (u_alphaMode == 1 && baseColor.a < u_alphaCutoff)
        discard;

    // ---------- Metallic / roughness ----------
    float metallic  = clamp(mrSample.b * u_metallicFactor, 0.0, 1.0);
    float roughness = clamp(mrSample.g * u_roughnessFactor, 0.0, 1.0);

    // ---------- Normal ----------
    vec3 N;
    if (u_hasNormalMap != 0)
    {
        vec3 T   = normalize(v_tangent.xyz);
        vec3 Ng  = normalize(v_normal);
        T        = normalize(T - dot(T, Ng) * Ng); // re-orthogonalise
        vec3 B   = cross(Ng, T) * v_tangent.w;
        mat3 TBN = mat3(T, B, Ng);
        N        = normalize(TBN * (normalSample * 2.0 - 1.0));
    }
    else
    {
        N = normalize(v_normal);
    }

    vec3  V     = normalize(u_eye.xyz - v_worldPos);
    float NdotV = max(dot(N, V), 0.0);

    // ---------- PBR material setup ----------
    vec3 dielectricSpec = vec3(0.04);
    vec3 F0             = mix(dielectricSpec, baseColor.rgb, metallic);
    vec3 albedo         = baseColor.rgb * (1.0 - metallic);

    // ---------- IBL — diffuse ----------
    vec3 F_ibl      = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kD         = (1.0 - F_ibl) * (1.0 - metallic);
    vec3 irradiance = texture(u_diffuseEnvMap, N).rgb;
    vec3 diffuse    = kD * albedo * irradiance;

    // ---------- IBL — specular (split-sum) ----------
    vec3  R                = reflect(-V, N);
    float lod              = roughness * u_roughnessScale;
    vec3  prefilteredColor = texture(u_specularEnvMap, vec4(R, lod)).rgb;
    vec2  brdf             = texture(u_brdfLUT, vec2(NdotV, roughness)).rg;
    vec3  specular         = prefilteredColor * (F_ibl * brdf.x + brdf.y);

    // ---------- Ambient occlusion ----------
    float ao = 1.0 + u_occlusionStrength * (aoSample - 1.0);

    // ---------- Emissive ----------
    vec3 emissive = emissiveSample * u_emissiveFactor;

    // ---------- Final ----------
    vec3 color = (diffuse + specular) * ao + emissive;

    fragColor = vec4(color, baseColor.a);
}
