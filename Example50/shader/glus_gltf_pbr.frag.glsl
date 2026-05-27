/*
 * Example 50 — PBR fragment shader.
 *
 * Working color space: linear Rec.2020 (BT.2020 primaries, D65 white).
 *
 * Inputs and the conversions we apply:
 *   - baseColor / emissive textures arrive as sRGB-encoded values authored
 *     against Rec.709 primaries.  GL_SRGB8_ALPHA8 hardware sampling already
 *     undoes the sRGB transfer, so the value coming out of texture() is
 *     linear Rec.709.  We multiply by REC709_TO_REC2020 to move it into the
 *     working color space.
 *   - The IBL cubemaps (diffuse, specular) and the brdf LUT are fed from a
 *     panorama that was pre-transformed Rec.709 -> Rec.2020 on the CPU
 *     before upload (see main.c:transformPanoramaToRec2020), so they are
 *     already in the working color space and need no further conversion.
 */

#version 460 core

uniform sampler2D u_baseColorTexture;
uniform sampler2D u_metallicRoughnessTexture;
uniform sampler2D u_normalTexture;
uniform sampler2D u_occlusionTexture;
uniform sampler2D u_emissiveTexture;

uniform samplerCubeArray u_specularEnvMap;
uniform samplerCube      u_diffuseEnvMap;
uniform sampler2D        u_brdfLUT;

uniform vec4  u_baseColorFactor;
uniform float u_metallicFactor;
uniform float u_roughnessFactor;
uniform vec3  u_emissiveFactor;
uniform float u_occlusionStrength;
uniform float u_alphaCutoff;
uniform int   u_alphaMode;
uniform int   u_hasNormalMap;

uniform float u_roughnessScale;

uniform vec4 u_eye;

in vec3 v_worldPos;
in vec3 v_normal;
in vec4 v_tangent;
in vec2 v_texCoord0;

out vec4 fragColor;

const float PI = 3.14159265358979323846;

// Linear Rec.709 (sRGB primaries) -> Linear Rec.2020 (BT.2020 primaries).
// Rows of the matrix in column-major GLSL form.
const mat3 REC709_TO_REC2020 = mat3(
    0.62740389896, 0.06909728935, 0.01639143887,
    0.32928303716, 0.91954039507, 0.08801330909,
    0.04331306388, 0.01136231558, 0.89559525204);

vec3 rec709_to_rec2020(vec3 c)
{
    return REC709_TO_REC2020 * c;
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main(void)
{
    vec4  baseColorSample = texture(u_baseColorTexture, v_texCoord0);
    vec4  mrSample        = texture(u_metallicRoughnessTexture, v_texCoord0);
    vec3  normalSample    = texture(u_normalTexture, v_texCoord0).xyz;
    float aoSample        = texture(u_occlusionTexture, v_texCoord0).r;
    vec3  emissiveSample  = texture(u_emissiveTexture, v_texCoord0).rgb;

    // Move authored colors from Rec.709 into Rec.2020 working space.
    baseColorSample.rgb = rec709_to_rec2020(baseColorSample.rgb);
    emissiveSample      = rec709_to_rec2020(emissiveSample);

    vec4 baseColor = baseColorSample * u_baseColorFactor;

    if (u_alphaMode == 1 && baseColor.a < u_alphaCutoff)
        discard;

    float metallic  = clamp(mrSample.b * u_metallicFactor, 0.0, 1.0);
    float roughness = clamp(mrSample.g * u_roughnessFactor, 0.0, 1.0);

    vec3 N;
    if (u_hasNormalMap != 0)
    {
        vec3 T   = normalize(v_tangent.xyz);
        vec3 Ng  = normalize(v_normal);
        T        = normalize(T - dot(T, Ng) * Ng);
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

    vec3 dielectricSpec = vec3(0.04);
    vec3 F0             = mix(dielectricSpec, baseColor.rgb, metallic);
    vec3 albedo         = baseColor.rgb * (1.0 - metallic);

    vec3 F_ibl      = fresnelSchlickRoughness(NdotV, F0, roughness);
    vec3 kD         = (1.0 - F_ibl) * (1.0 - metallic);
    vec3 irradiance = texture(u_diffuseEnvMap, N).rgb;
    vec3 diffuse    = kD * albedo * irradiance;

    vec3  R                = reflect(-V, N);
    float lod              = roughness * u_roughnessScale;
    vec3  prefilteredColor = texture(u_specularEnvMap, vec4(R, lod)).rgb;
    vec2  brdf             = texture(u_brdfLUT, vec2(NdotV, roughness)).rg;
    vec3  specular         = prefilteredColor * (F_ibl * brdf.x + brdf.y);

    float ao = 1.0 + u_occlusionStrength * (aoSample - 1.0);

    vec3 emissive = emissiveSample * u_emissiveFactor;

    vec3 color = (diffuse + specular) * ao + emissive;

    fragColor = vec4(color, baseColor.a);
}
