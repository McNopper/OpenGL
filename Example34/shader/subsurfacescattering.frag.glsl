#version 150

uniform vec4 u_diffuseColor;
uniform vec4 u_scatterColor;
uniform vec3 u_lightDirection;
uniform sampler2DShadow u_depthPassTexture;

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

float getDepthPassSpaceZ(float z)
{
	float n = u_nearFar.x;
	float f = u_nearFar.y;

	return -2*f*n / (z*(n-f)+f+n);
}

void main(void)
{
	float zInNDC = textureProj(u_depthPassTexture, v_projCoord);	
	float zOutNDC = v_projCoord.z / v_projCoord.w;
	
	float zIn = getDepthPassSpaceZ(zInNDC); 
	float zOut = getDepthPassSpaceZ(zOutNDC);
	
	float scatterFalloff = exp(-abs(zOut - zIn) * u_scatterFalloff);
	
	//
	
	vec3 normal = normalize(v_normal);	

	float NdotL = dot(normal, u_lightDirection);		
	float NdotL_wrap = wrapLighting(NdotL, u_wrap);
	
	float diffuseIntensity = max(NdotL, 0.0);		
	float scatterIntensity = scatterTint(NdotL_wrap, u_scatterWidth);

	fragColor = u_diffuseColor * diffuseIntensity + u_scatterColor * scatterIntensity * scatterFalloff;
}
