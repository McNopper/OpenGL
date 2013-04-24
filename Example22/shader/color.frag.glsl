#version 150

uniform vec4 u_shapeColor;
uniform vec3 u_lightDirection;

in vec3 v_normal;
in vec3 v_eye;

out vec4 fragColor;

void main(void)
{
	vec4 color;

	vec3 normal = normalize(v_normal);	

	float diffuseIntensity = max(dot(normal, u_lightDirection), 0.0);

	color = u_shapeColor*0.3;

	if (diffuseIntensity > 0.0)
	{
		color += u_shapeColor*diffuseIntensity;
	
		vec3 reflection = reflect(-u_lightDirection, normal);

		vec3 eye = normalize(v_eye);

		float specularItensity = pow(max(dot(reflection, eye), 0.0), 20.0);
		
		color += vec4(1.0, 1.0, 1.0, 1.0)*specularItensity;
	}

	fragColor = color;
}
