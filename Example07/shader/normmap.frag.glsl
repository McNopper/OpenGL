#version 150

uniform sampler2D u_texture; 
uniform sampler2D u_normalMap; 

in vec2 v_texCoord;
in vec3 v_tsLight;
in vec3 v_tsEye;

out vec4 fragColor;

void main(void)
{
	// Note: All calculations are in texture space.

	vec4 textureColor = texture(u_texture, v_texCoord);

	// Fixed 0.3 ambient light.
	vec4 color = 0.3 * textureColor;

	// Convert the color information to a normal.
	vec3 normalDX = normalize( texture(u_normalMap, v_texCoord).xyz * 2.0 - 1.0 );
	// DirectX: Origin of uv is upper left. OpenGL: Origin of st is lower left. Normal vector points "towards" us in both cases.
	// As normal maps usually are created for DirectX, we have to convert from DirectX to OpenGL tangent space.
	vec3 normal;
	normal.x = dot(vec3(1.0, 0.0, 0.0), normalDX);
	normal.y = dot(vec3(0.0, -1.0, 0.0), normalDX);
	normal.z = dot(vec3(0.0, 0.0, 1.0), normalDX);
	
	vec3 light = normalize(v_tsLight);

	float nDotL = max(dot(light, normal), 0.0);
		
	if (nDotL > 0.0)
	{
		vec3 eye = normalize(v_tsEye);
	
		// Incident vector is opposite light direction vector.
		vec3 reflection = reflect(-light, normal);
		
		float eDotR = max(dot(eye, reflection), 0.0);
	
		color += textureColor * nDotL;
		
		// Fixed 0.1 specular color factor and fixed 20.0 specular exponent.
		color += vec4(0.1, 0.1, 0.1, 0.1) * pow(eDotR, 20.0);
	}

	fragColor = color;	
}
