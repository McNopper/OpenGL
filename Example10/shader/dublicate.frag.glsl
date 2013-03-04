#version 150

in vec3 v_normal;
in vec3 v_eye;
in vec4 v_color;
in vec3 v_lightDirection;

out vec4 fragColor;

void main(void)
{
	// Note: All calculations are in camera space.

	vec4 color = 0.3 * v_color;

	vec3 normal = normalize(v_normal);
	
	vec3 lightDirection = normalize(v_lightDirection);

	float nDotL = max(dot(lightDirection, normal), 0.0);
		
	if (nDotL > 0.0)
	{
		vec3 eye = normalize(v_eye);
	
		// Incident vector is opposite light direction vector.
		vec3 reflection = reflect(-lightDirection, normal);
		
		float eDotR = max(dot(eye, reflection), 0.0);
	
		color += 1.0 * v_color * nDotL;
		
		color += 1.0 * vec4(1.0, 1.0, 1.0, 1.0) * pow(eDotR, 20.0);
	}

	fragColor = color;
}
