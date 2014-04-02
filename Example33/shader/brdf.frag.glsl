#version 410 core

uniform samplerCubeArray u_textureSpecular;
uniform samplerCube u_textureDiffuse;
uniform sampler2D u_textureLUT;

uniform vec3 u_colorMaterial;
uniform float u_roughnessMaterial;
uniform float u_roughnessScale;
uniform float u_R0Material;

in vec3 v_eye;
in vec3 v_normal;

out vec4 fragColor;

// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
vec3 brdfLambert(vec3 ray)
{
	return u_colorMaterial * texture(u_textureDiffuse, ray).rgb;
}

// see http://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
vec3 brdfCookTorrance(vec3 N, vec3 V)
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
		float rLow = floor(scaledRoughness);
		float rHigh = ceil(scaledRoughness);	
		float rFraction = scaledRoughness - rLow;
		
		// see page 6 and 7 of the above document
		vec3 prefilteredColor = mix(texture(u_textureSpecular, vec4(L, rLow)).rgb, texture(u_textureSpecular, vec4(L, rHigh)).rgb, rFraction);

		vec2 envBRDF = texture(u_textureLUT, vec2(NdotV, u_roughnessMaterial)).rg;
		
		return prefilteredColor * (u_R0Material * envBRDF.x + envBRDF.y);
	}
	
	return noColor;
}

void main(void)
{
	vec3 color = vec3(0.0, 0.0, 0.0);
	
	vec3 eye = normalize(v_eye);
	
	vec3 normal = normalize(v_normal);

	// Diffuse
	color += brdfLambert(normal);
		
	// Specular
	color += brdfCookTorrance(normal, eye);
	
	fragColor = vec4(color, 1.0);
}
