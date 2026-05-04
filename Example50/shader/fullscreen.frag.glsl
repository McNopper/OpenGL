/*
 * Example 50 — fullscreen resolve / output encoding shader.
 *
 * The MSAA color buffer holds linear scene-referred values in Rec.2020
 * primaries.  This shader resolves the multisamples and converts to the
 * encoding required by whatever EGL_GL_COLORSPACE the surface was created
 * with.  Everything else in the renderer is colorspace-agnostic.
 *
 * Output colorspace IDs (must match main.c):
 *   0  OUT_LINEAR             linear Rec.709, no transfer (SDR fallback)
 *   1  OUT_SRGB               Rec.709 + sRGB transfer
 *   2  OUT_DISPLAY_P3         Display P3 + sRGB transfer
 *   3  OUT_DISPLAY_P3_LINEAR  Display P3, linear
 *   4  OUT_SCRGB_LINEAR       Rec.709 linear, scRGB scale (1.0 = 80 nits)
 *   5  OUT_SCRGB              Rec.709 + scRGB gamma, extended range
 *   6  OUT_BT2020_LINEAR      Rec.2020 linear (1.0 = 10000 nits)
 *   7  OUT_BT2020_HLG         Rec.2020 + HLG OETF
 *   8  OUT_BT2020_PQ          Rec.2020 + PQ (HDR10)
 *
 * Reference luminance:
 *   A scene-referred linear value of 1.0 in working space is treated as
 *   u_referenceWhiteNits cd/m^2 on the display side.  HDR highlights above
 *   1.0 scale linearly up to u_peakNits (the panel's published peak); above
 *   peak we soft-clip with a simple Reinhard-style roll-off so headroom is
 *   preserved without hard clipping.
 */

#version 460 core

uniform sampler2DMS u_framebufferTexture;
uniform int   u_msaaSamples;
uniform float u_gamma;
uniform int   u_outputColorspace;
uniform float u_referenceWhiteNits;
uniform float u_peakNits;

in  vec2 v_texCoord;
out vec4 fragColor;

const int OUT_LINEAR            = 0;
const int OUT_SRGB              = 1;
const int OUT_DISPLAY_P3        = 2;
const int OUT_DISPLAY_P3_LINEAR = 3;
const int OUT_SCRGB_LINEAR      = 4;
const int OUT_SCRGB             = 5;
const int OUT_BT2020_LINEAR     = 6;
const int OUT_BT2020_HLG        = 7;
const int OUT_BT2020_PQ         = 8;

// Linear Rec.2020 -> Linear Rec.709 (D65, Bradford-free).
const mat3 REC2020_TO_REC709 = mat3(
	 1.66049100210, -0.12455047793, -0.01815076131,
	-0.58764114862,  1.13289989357, -0.10057889400,
	-0.07284985348, -0.00834941564,  1.11872965531);

// Linear Rec.2020 -> Linear Display P3 (D65).
const mat3 REC2020_TO_DISPLAY_P3 = mat3(
	 1.34357825801, -0.06529745278, -0.02807139649,
	-0.28217967052,  1.07578791321, -0.19597391244,
	-0.06139858748, -0.01049046043,  1.22404530893);

vec3 toRec709(vec3 c)       { return REC2020_TO_REC709    * c; }
vec3 toDisplayP3(vec3 c)    { return REC2020_TO_DISPLAY_P3 * c; }

// Soft-clip luminance above the panel peak so we keep all detail.
vec3 softShoulder(vec3 nits, float peak)
{
	vec3 x = max(nits, vec3(0.0)) / peak;
	vec3 y = x / (1.0 + x);                  // Reinhard
	return y * peak * (1.0 + 1.0 / peak);    // re-normalize so x=peak ~ peak
}

// sRGB OETF (per channel).
float srgbEncode(float c)
{
	return (c <= 0.0031308) ? (12.92 * c)
	                        : (1.055 * pow(c, 1.0 / 2.4) - 0.055);
}
vec3 srgbEncode3(vec3 c)
{
	return vec3(srgbEncode(c.r), srgbEncode(c.g), srgbEncode(c.b));
}

// scRGB gamma is sRGB-style but sign-symmetric, applied to extended-range fp16.
float scrgbEncode(float c)
{
	float s = sign(c);
	float a = abs(c);
	return s * srgbEncode(a);
}
vec3 scrgbEncode3(vec3 c)
{
	return vec3(scrgbEncode(c.r), scrgbEncode(c.g), scrgbEncode(c.b));
}

// SMPTE ST 2084 PQ OETF.  Input: normalized 0..1 corresponding to 0..10000 nits.
float pqEncode(float Y)
{
	const float m1 = 2610.0 / 16384.0;
	const float m2 = 2523.0 /  4096.0 * 128.0;
	const float c1 = 3424.0 /  4096.0;
	const float c2 = 2413.0 /  4096.0 *  32.0;
	const float c3 = 2392.0 /  4096.0 *  32.0;
	float Ym = pow(clamp(Y, 0.0, 1.0), m1);
	return pow((c1 + c2 * Ym) / (1.0 + c3 * Ym), m2);
}
vec3 pqEncode3(vec3 nits10k)
{
	return vec3(pqEncode(nits10k.r), pqEncode(nits10k.g), pqEncode(nits10k.b));
}

// ARIB STD-B67 / BT.2100 HLG OETF.  Input: scene-linear normalized 0..1.
float hlgEncode(float E)
{
	const float a = 0.17883277;
	const float b = 0.28466892;
	const float c = 0.55991073;
	float x = clamp(E, 0.0, 1.0);
	return (x <= 1.0/12.0) ? sqrt(3.0 * x)
	                       : a * log(12.0 * x - b) + c;
}
vec3 hlgEncode3(vec3 E) { return vec3(hlgEncode(E.r), hlgEncode(E.g), hlgEncode(E.b)); }

// Classic SDR exposure tone-map (preserved from Example49 for SDR paths).
vec3 sdrTonemap(vec3 hdr)
{
	return vec3(1.0) - exp2(-hdr);
}

void main(void)
{
	ivec2 texelCoord = ivec2(v_texCoord * textureSize(u_framebufferTexture));

	vec3 hdrRec2020 = vec3(0.0);
	for (int i = 0; i < u_msaaSamples; ++i)
		hdrRec2020 += texelFetch(u_framebufferTexture, texelCoord, i).rgb;
	hdrRec2020 /= float(u_msaaSamples);

	vec3 outColor = vec3(0.0);

	if (u_outputColorspace == OUT_BT2020_PQ)
	{
		vec3 nits = softShoulder(hdrRec2020 * u_referenceWhiteNits, u_peakNits);
		outColor  = pqEncode3(nits / 10000.0);
	}
	else if (u_outputColorspace == OUT_BT2020_HLG)
	{
		vec3 nits = softShoulder(hdrRec2020 * u_referenceWhiteNits, u_peakNits);
		outColor  = hlgEncode3(nits / u_peakNits);
	}
	else if (u_outputColorspace == OUT_BT2020_LINEAR)
	{
		vec3 nits = softShoulder(hdrRec2020 * u_referenceWhiteNits, u_peakNits);
		outColor  = clamp(nits / 10000.0, 0.0, 1.0);
	}
	else if (u_outputColorspace == OUT_SCRGB_LINEAR)
	{
		// scRGB: linear Rec.709, fp16 extended range, 1.0 == 80 nits.
		vec3 rec709 = toRec709(hdrRec2020);
		outColor    = rec709 * (u_referenceWhiteNits / 80.0);
	}
	else if (u_outputColorspace == OUT_SCRGB)
	{
		vec3 rec709 = toRec709(hdrRec2020) * (u_referenceWhiteNits / 80.0);
		outColor    = scrgbEncode3(rec709);
	}
	else if (u_outputColorspace == OUT_DISPLAY_P3)
	{
		vec3 p3 = toDisplayP3(sdrTonemap(hdrRec2020));
		outColor = srgbEncode3(clamp(p3, 0.0, 1.0));
	}
	else if (u_outputColorspace == OUT_DISPLAY_P3_LINEAR)
	{
		outColor = clamp(toDisplayP3(sdrTonemap(hdrRec2020)), 0.0, 1.0);
	}
	else if (u_outputColorspace == OUT_LINEAR)
	{
		outColor = clamp(toRec709(sdrTonemap(hdrRec2020)), 0.0, 1.0);
	}
	else // OUT_SRGB and any unknown id fall back here.
	{
		vec3 rec709 = clamp(toRec709(sdrTonemap(hdrRec2020)), 0.0, 1.0);
		outColor    = pow(rec709, vec3(1.0 / u_gamma));
	}

	fragColor = vec4(outColor, 1.0);
}
