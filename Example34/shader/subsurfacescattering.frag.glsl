#version 150

uniform vec4 u_diffuseColor;
uniform vec4 u_scatterColor;
uniform vec3 u_lightDirection;
uniform sampler2D u_depthPassTexture;

uniform vec2 u_nearFar;
uniform float u_wrap;
uniform float u_scatterWidth;
uniform float u_scatterFalloff;

in vec3 v_normal;
in vec4 v_projCoord;

out vec4 fragColor;

// see http://http.developer.nvidia.com/GPUGems/gpugems_ch16.html

float wrapLighting(float x, float wrap)
{
	return (x + wrap) / (1.0 + wrap);
}

float scatterTint(float x, float scatterWidth)
{
	if (scatterWidth == 0.0)
	{
		return 0.0;
	}

	return smoothstep(0.0, scatterWidth, x) * smoothstep(scatterWidth * 2.0, scatterWidth, x); 
}

float getDepthPassSpaceZ(float zWC)
{
	float n = u_nearFar.x;
	float f = u_nearFar.y;
	
	// Assume standard depth range [0..1]
	float zNDC = (zWC - 0.5) * 2.0;

	return -2*f*n / (zNDC*(n-f)+f+n);
}

void main(void)
{
	float zInWC = texture(u_depthPassTexture, v_projCoord.xy / v_projCoord.w).r;
	float zOutWC = v_projCoord.z / v_projCoord.w;
	
	float zIn = getDepthPassSpaceZ(zInWC); 
	float zOut = getDepthPassSpaceZ(zOutWC);
	
	float scatterFalloff = exp(-abs(zOut - zIn) * u_scatterFalloff);
	
	//
	
	vec3 normal = normalize(v_normal);	

	float NdotL = dot(normal, u_lightDirection);		
	float NdotL_wrap = wrapLighting(NdotL, u_wrap);
	
	float diffuseIntensity = max(NdotL, 0.0);		
	float scatterIntensity = scatterTint(NdotL_wrap, u_scatterWidth);

	fragColor = u_diffuseColor * diffuseIntensity + u_scatterColor * scatterIntensity * scatterFalloff;
}
