#version 150

const float Eta = 0.15; // Water

uniform samplerCube u_cubemap;
uniform sampler2D u_waterTexture; 

in vec3 v_incident;

in vec3 v_bitangent;
in vec3 v_normal;
in vec3 v_tangent;

in vec2 v_texCoord;

out vec4 fragColor;

vec3 textureToNormal(vec4 orgNormalColor)
{
	return normalize(vec3(clamp(orgNormalColor.r*2.0 - 1.0, -1.0, 1.0), clamp(orgNormalColor.g*2.0 - 1.0, -1.0, 1.0), clamp(orgNormalColor.b*2.0 - 1.0, -1.0, 1.0)));
}

void main(void)
{
	// The normals stored in the texture are in object space. No world transformations are yet done.
	vec3 objectNormal = textureToNormal(texture(u_waterTexture, v_texCoord));

	// These three vectors span a basis depending on the world transformations e.g. the waves.
	mat3 objectToWorldMatrix = mat3(normalize(v_bitangent), normalize(v_normal), normalize(v_tangent));

	vec3 worldNormal = objectToWorldMatrix * objectNormal;

	vec3 worldIncident = normalize(v_incident);
	
	vec3 refraction = refract(worldIncident, worldNormal, Eta);
	vec3 reflection = reflect(worldIncident, worldNormal);
	
	vec4 refractionColor = texture(u_cubemap, refraction);
	vec4 reflectionColor = texture(u_cubemap, reflection);
	
	float fresnel = Eta + (1.0 - Eta) * pow(max(0.0, 1.0 - dot(-worldIncident, worldNormal)), 5.0);
				
	fragColor = mix(refractionColor, reflectionColor, fresnel);
}
