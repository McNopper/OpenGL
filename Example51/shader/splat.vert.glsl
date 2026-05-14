#version 460 core

// SPLAT_STRIDE and SH_DEGREE are injected at compile time.
#ifndef SPLAT_STRIDE
#define SPLAT_STRIDE 59
#endif
#ifndef SH_DEGREE
#define SH_DEGREE 3
#endif

// 3-sigma splat extent (KHR_gaussian_splatting §Rendering).
const float SD = 3.0;

// SH basis constants with Condon-Shortley phase (KHR_gaussian_splatting §Appendix A).
const float C0 =  0.28209479177387814f;
const float C1 =  0.4886025119029199f;
const float C2a = 1.0925484305920792f;
const float C2b = 0.31539156525252005f;
const float C2c = 0.5462742152960396f;
const float C3a = 0.5900435899266435f;
const float C3b = 2.890611442640554f;
const float C3c = 0.4570457994644658f;
const float C3d = 0.3731763325901154f;
const float C3e = 1.4453057213303264f;

layout(location = 0) in vec2 a_quadEdge;

layout(std140, binding = 0) uniform WorldData {
    mat4 projMatrix;
    mat4 viewMatrix;
    vec2 focal;
    vec2 viewport;
    vec4 camPos;
} world;

layout(std430, binding = 0) readonly buffer SplatBuffer { float splats[]; };
layout(std430, binding = 1) readonly buffer IndexBuffer { uint  indices[]; };

layout(std430, binding = 3) readonly buffer ModelBuffer {
    mat4  worldMatrix;
    float wigner1[9];   // column-major 3×3
    float wigner2[25];  // column-major 5×5
    float wigner3[49];  // column-major 7×7
};

out vec3  v_color;
out float v_opacity;
out vec2  v_u;

// Convert a unit quaternion (xyzw) to a 3x3 rotation matrix (column-major).
mat3 quatToMat3(vec4 q)
{
    float x = q.x, y = q.y, z = q.z, w = q.w;
    return mat3(
        1.0 - 2.0*(y*y + z*z),  2.0*(x*y + z*w),         2.0*(x*z - y*w),
              2.0*(x*y - z*w),  1.0 - 2.0*(x*x + z*z),   2.0*(y*z + x*w),
              2.0*(x*z + y*w),        2.0*(y*z - x*w),    1.0 - 2.0*(x*x + y*y)
    );
}

// Apply a Wigner-D band-1 rotation (column-major 3x3) to 3 RGB SH coefficients.
void rotateSH1(inout vec3 c[3])
{
    int  mp, m;
    vec3 tmp[3];

    for (mp = 0; mp < 3; mp++)
    {
        tmp[mp] = vec3(0.0);
        for (m = 0; m < 3; m++)
            tmp[mp] += wigner1[m * 3 + mp] * c[m];
    }
    for (mp = 0; mp < 3; mp++)
        c[mp] = tmp[mp];
}

// Apply a Wigner-D band-2 rotation (column-major 5x5) to 5 RGB SH coefficients.
void rotateSH2(inout vec3 c[5])
{
    int  mp, m;
    vec3 tmp[5];

    for (mp = 0; mp < 5; mp++)
    {
        tmp[mp] = vec3(0.0);
        for (m = 0; m < 5; m++)
            tmp[mp] += wigner2[m * 5 + mp] * c[m];
    }
    for (mp = 0; mp < 5; mp++)
        c[mp] = tmp[mp];
}

// Apply a Wigner-D band-3 rotation (column-major 7x7) to 7 RGB SH coefficients.
void rotateSH3(inout vec3 c[7])
{
    int  mp, m;
    vec3 tmp[7];

    for (mp = 0; mp < 7; mp++)
    {
        tmp[mp] = vec3(0.0);
        for (m = 0; m < 7; m++)
            tmp[mp] += wigner3[m * 7 + mp] * c[m];
    }
    for (mp = 0; mp < 7; mp++)
        c[mp] = tmp[mp];
}

void main()
{
    uint  splatIdx = indices[gl_InstanceID];
    uint  base     = splatIdx * uint(SPLAT_STRIDE);

    vec3  pos     = vec3(splats[base], splats[base + 1u], splats[base + 2u]);
    vec4  quat    = vec4(splats[base + 3u], splats[base + 4u], splats[base + 5u], splats[base + 6u]);
    vec3  scale   = vec3(splats[base + 7u], splats[base + 8u], splats[base + 9u]);
    float opacity = splats[base + 10u];

    vec3  worldPos = (worldMatrix * vec4(pos, 1.0)).xyz;
    vec4  clipPos  = world.projMatrix * world.viewMatrix * vec4(worldPos, 1.0);
    vec3  viewPos  = (world.viewMatrix * vec4(worldPos, 1.0)).xyz;

    if (viewPos.z >= -0.001)
    {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        v_color   = vec3(0.0);
        v_opacity = 0.0;
        v_u       = vec2(0.0);
        return;
    }

    //
    // 2D covariance projection (KHR_gaussian_splatting, Covariance section).
    // Sigma = M * R * diag(s)^2 * R^T * M^T
    //
    mat3 M  = mat3(worldMatrix);
    mat3 R  = quatToMat3(quat);
    mat3 RS = mat3(R[0] * scale.x, R[1] * scale.y, R[2] * scale.z);
    mat3 sigma3D = M * RS * transpose(RS) * transpose(M);

    float fx = world.focal.x, fy = world.focal.y;
    float tz = viewPos.z, tx = viewPos.x, ty = viewPos.y;
    mat3 J = mat3(
        fx / tz,           0.0,                0.0,
        0.0,               fy / tz,            0.0,
        -fx * tx / (tz*tz), -fy * ty / (tz*tz), 0.0
    );

    mat3 W  = mat3(world.viewMatrix);
    mat3 T  = J * W;
    mat3 c3 = T * sigma3D * transpose(T);
    mat2 cov2D = mat2(c3[0].xy, c3[1].xy);

    // Low-pass filter (Kerbl et al. 2023, Appendix A).
    cov2D[0][0] += 0.3;
    cov2D[1][1] += 0.3;

    float a = cov2D[0][0], b = cov2D[0][1], c = cov2D[1][1];
    float trace = a + c;
    float det   = a * c - b * b;
    float disc  = sqrt(max(0.0, trace * trace * 0.25 - det));
    float lambda1 = trace * 0.5 + disc;
    float lambda2 = trace * 0.5 - disc;

    if (lambda1 <= 0.0 || lambda2 <= 0.0)
    {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        v_color   = vec3(0.0);
        v_opacity = 0.0;
        v_u       = vec2(0.0);
        return;
    }

    // Discard splats whose footprint exceeds the viewport (KHR_gaussian_splatting §Rendering).
    if (sqrt(lambda1) * SD > min(world.viewport.x, world.viewport.y))
    {
        gl_Position = vec4(2.0, 2.0, 2.0, 1.0);
        v_color   = vec3(0.0);
        v_opacity = 0.0;
        v_u       = vec2(0.0);
        return;
    }

    vec2 ev1= (abs(b) > 1e-5) ? normalize(vec2(b, lambda1 - a))
                                 : ((a > c) ? vec2(1.0, 0.0) : vec2(0.0, 1.0));
    vec2 ev2 = vec2(-ev1.y, ev1.x);

    vec2 axis1 = ev1 * sqrt(lambda1) * SD;
    vec2 axis2 = ev2 * sqrt(lambda2) * SD;
    vec2 screenOffset = a_quadEdge.x * axis1 + a_quadEdge.y * axis2;
    vec2 ndcPos = clipPos.xy / clipPos.w + screenOffset * 2.0 / world.viewport;

    gl_Position = vec4(ndcPos * clipPos.w, clipPos.zw);

    v_u = a_quadEdge * SD;

    //
    // Spherical harmonics colour evaluation (KHR_gaussian_splatting, SH section).
    //
    vec3 dir = normalize(world.camPos.xyz - worldPos);
    float x = dir.x, y = dir.y, z = dir.z;

    vec3 sh0 = vec3(splats[base + 11u], splats[base + 12u], splats[base + 13u]);
    vec3 color = sh0 * C0 + 0.5;

#if SH_DEGREE >= 1
    vec3 sh1[3];
    sh1[0] = vec3(splats[base+14u], splats[base+15u], splats[base+16u]);
    sh1[1] = vec3(splats[base+17u], splats[base+18u], splats[base+19u]);
    sh1[2] = vec3(splats[base+20u], splats[base+21u], splats[base+22u]);

    rotateSH1(sh1);

    color += sh1[0] * (-C1 * y)
           + sh1[1] * ( C1 * z)
           + sh1[2] * (-C1 * x);
#endif

#if SH_DEGREE >= 2
    vec3 sh2[5];
    sh2[0] = vec3(splats[base+23u], splats[base+24u], splats[base+25u]);
    sh2[1] = vec3(splats[base+26u], splats[base+27u], splats[base+28u]);
    sh2[2] = vec3(splats[base+29u], splats[base+30u], splats[base+31u]);
    sh2[3] = vec3(splats[base+32u], splats[base+33u], splats[base+34u]);
    sh2[4] = vec3(splats[base+35u], splats[base+36u], splats[base+37u]);

    rotateSH2(sh2);

    color += sh2[0] * ( C2a * x * y)
           + sh2[1] * (-C2a * y * z)
           + sh2[2] * ( C2b * (3.0*z*z - 1.0))
           + sh2[3] * (-C2a * x * z)
           + sh2[4] * ( C2c * (x*x - y*y));
#endif

#if SH_DEGREE >= 3
    vec3 sh3[7];
    sh3[0] = vec3(splats[base+38u], splats[base+39u], splats[base+40u]);
    sh3[1] = vec3(splats[base+41u], splats[base+42u], splats[base+43u]);
    sh3[2] = vec3(splats[base+44u], splats[base+45u], splats[base+46u]);
    sh3[3] = vec3(splats[base+47u], splats[base+48u], splats[base+49u]);
    sh3[4] = vec3(splats[base+50u], splats[base+51u], splats[base+52u]);
    sh3[5] = vec3(splats[base+53u], splats[base+54u], splats[base+55u]);
    sh3[6] = vec3(splats[base+56u], splats[base+57u], splats[base+58u]);

    rotateSH3(sh3);

    color += sh3[0] * (-C3a * y * (3.0*x*x - y*y))
           + sh3[1] * ( C3b * x * y * z)
           + sh3[2] * (-C3c * y * (5.0*z*z - 1.0))
           + sh3[3] * ( C3d * z * (5.0*z*z - 3.0))
           + sh3[4] * (-C3c * x * (5.0*z*z - 1.0))
           + sh3[5] * ( C3e * z * (x*x - y*y))
           + sh3[6] * (-C3a * x * (x*x - 3.0*y*y));
#endif

    v_color  = max(color, vec3(0.0));
    v_opacity = opacity;
}
