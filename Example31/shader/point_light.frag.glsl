#version 410 core

uniform sampler2D u_diffuse;
uniform sampler2D u_specular;
uniform sampler2D u_position;
uniform sampler2D u_normal;

uniform mat4 u_biasMatrix;

uniform float u_lightRadius;

flat in vec4 v_lightPosition;
flat in vec4 v_lightColor;
in vec4 v_lightBorderPosition;

out vec4 fragColor;

void main()
{
	vec4 lightBorderPosition = v_lightBorderPosition; 
	lightBorderPosition /= lightBorderPosition.w;
	
	// Do bias transformation in fragment shader. Otherwise close point lights to the camera do cause artifacts. 	
	vec2 texCoord = (u_biasMatrix * lightBorderPosition).st;

	vec4 diffuseColor = vec4(texture(u_diffuse, texCoord).rgb, 1.0);
	vec4 specularColor = vec4(texture(u_specular, texCoord).rgb, 1.0);
	float shininess = texture(u_position, texCoord).a;
	vec4 position = vec4(texture(u_position, texCoord).rgb, 1.0);
	vec3 normal = texture(u_normal, texCoord).rgb;
	
	// Eye is at origin, as we do calculations in camera space.
	vec3 eyeDirection = normalize(-position.xyz);
	
	vec3 lightVector = v_lightPosition.xyz - position.xyz;
	
	// Linear attenuate to the border of the point light.
	float attenuation = u_lightRadius - clamp(length(lightVector), 0.0, u_lightRadius);
	
	float diffuseIntensity = 0.0;
	
	float specularIntensity = 0.0;
	
	if (attenuation > 0.0)
	{		
		vec3 lightDirection = normalize(lightVector);
		
		diffuseIntensity = max(dot(normal, lightDirection), 0.0);
			
		float eDotR = max(dot(reflect(-lightDirection, normal), eyeDirection), 0.0);
			
		if (eDotR > 0.0)
		{
			specularIntensity = pow(eDotR, shininess);
		}
	}
	
	fragColor = v_lightColor * diffuseColor * diffuseIntensity * attenuation + v_lightColor * specularColor * specularIntensity * attenuation;
	
	//fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}   