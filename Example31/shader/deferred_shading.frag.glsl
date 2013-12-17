#version 410 core

struct MaterialProperties
{
	vec4 diffuseColor;
	vec4 specularColor;
	float specularExponent;
	sampler2D diffuseTexture;
};

uniform	MaterialProperties u_material;

uniform	int u_useTexture;

in vec4 v_position;
in vec3 v_normal;
in vec2 v_texCoord;

layout(location = 0) out vec3 diffuse;
layout(location = 1) out vec3 specular;
layout(location = 2) out vec4 position;
layout(location = 3) out vec3 normal;

void main()
{
	// Note: All calculations are stored in camera space.

	position = v_position;
	
	normal = normalize(v_normal);

	vec4 textureColor = vec4(1.0, 1.0, 1.0, 1.0);
	if (u_useTexture != 0)
	{
		textureColor = texture(u_material.diffuseTexture, v_texCoord);
	}

	diffuse = textureColor.rgb;
	
	specular = u_material.specularColor.rgb;
	
	// We store shininess in w of the position 
	position.w = u_material.specularExponent;
}   