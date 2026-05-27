#version 410 core

uniform samplerCubeArray u_textureSpecular;
uniform samplerCube      u_textureDiffuse;
uniform sampler2D        u_textureLUT;

uniform vec3  u_colorMaterial;
uniform float u_roughnessMaterial;
uniform float u_roughnessScale;
uniform float u_R0Material;

in vec3 v_eye;
in vec3 v_normal;

out vec4 fragColor;

// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
// Diffuse is not Fresnel-attenuated here, matching Example32 which sums diffuse + specular directly.
vec3 brdfLambert(vec3 N)
{
    return u_colorMaterial * texture(u_textureDiffuse, N).rgb;
}

// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
vec3 brdfCookTorrance(vec3 N, vec3 V, float F_ibl)
{
    vec3 noColor = vec3(0.0, 0.0, 0.0);

    // Note: reflect takes incident vector.
    // Note: Use N instead of H for approximation.
    vec3 L = reflect(-V, N);

    float NdotL = dot(N, L);
    float NdotV = dot(N, V);

    // Lighted and visible
    if (NdotL > 0.0 && NdotV > 0.0)
    {
        float scaledRoughness = u_roughnessMaterial * u_roughnessScale;
        float rLow            = floor(scaledRoughness);
        float rHigh           = ceil(scaledRoughness);
        float rFraction       = scaledRoughness - rLow;

        // see page 6 and 7 of the above document
        vec3 prefilteredColor = mix(texture(u_textureSpecular, vec4(L, rLow)).rgb, texture(u_textureSpecular, vec4(L, rHigh)).rgb, rFraction);

        return prefilteredColor * F_ibl;
    }

    return noColor;
}

void main(void)
{
    vec3 color = vec3(0.0, 0.0, 0.0);

    vec3 eye = normalize(v_eye);

    vec3 normal = normalize(v_normal);

    // Clamp NdotV to avoid out-of-range LUT reads at grazing angles.
    float NdotV = clamp(dot(normal, eye), 0.0, 1.0);

    vec2 envBRDF = texture(u_textureLUT, vec2(NdotV, u_roughnessMaterial)).rg;

    // Roughness-dependent Fresnel from Fdez-Aguera, as used in the glTF reference renderer.
    // see https://bruop.github.io/ibl/#single_scattering_results
    // At high roughness the grazing Fresnel boost is suppressed, matching physical expectation.
    float Fr     = max(1.0 - u_roughnessMaterial, u_R0Material) - u_R0Material;
    float k_S    = u_R0Material + Fr * pow(1.0 - NdotV, 5.0);
    float FssEss = k_S * envBRDF.x + envBRDF.y;

    // Multiple-scattering compensation (Fdez-Aguera 2019): recover energy lost when a
    // ray bounces more than once between microfacets, which single-scattering BRDFs ignore.
    float Ems    = 1.0 - (envBRDF.x + envBRDF.y);
    float F_avg  = u_R0Material + (1.0 - u_R0Material) / 21.0;
    float FmsEms = Ems * FssEss * F_avg / (1.0 - F_avg * Ems);

    // Combined IBL Fresnel weight used for both energy conservation and specular scaling.
    float F_ibl = FssEss + FmsEms;

    // Diffuse: full weight, consistent with Example32's non-attenuated Lambertian.
    color += brdfLambert(normal);

    // Specular
    color += brdfCookTorrance(normal, eye, F_ibl);

    fragColor = vec4(color, 1.0);
}
