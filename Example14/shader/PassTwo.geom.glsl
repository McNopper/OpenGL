#version 410 core

layout(triangles, invocations = 1) in;
layout(triangle_strip, max_vertices = 3) out;

uniform sampler2DRect u_heightMapTexture;
uniform sampler2DRect u_normalMapTexture;

uniform mat4 u_tmvpMatrix;

uniform vec3 u_lightDirection;

out vec2 v_texCoord;

out float v_intensity;

void main(void)
{
	ivec2 heightMapTextureSize = textureSize(u_heightMapTexture);

	vec4 heightMapPosition;
	
	vec3 normal;
	
	for(int i = 0; i < gl_in.length(); ++i)
	{
		heightMapPosition = gl_in[i].gl_Position;
		
		heightMapPosition.y = texture(u_heightMapTexture, heightMapPosition.xz).r;

		v_texCoord = vec2((heightMapPosition.x - 0.5) / heightMapTextureSize.s, (heightMapPosition.z - 0.5) / heightMapTextureSize.t);
		
		// Normals in the normal map represent world space coordinates ...
		normal = texture(u_normalMapTexture, heightMapPosition.xz).xyz * 2.0 - 1.0;
				
		// ... so calculations are in world space 
		v_intensity = max(dot(normalize(normal), u_lightDirection), 0.0);		
		
		gl_Position = u_tmvpMatrix*heightMapPosition;
		
		EmitVertex();
	}

	EndPrimitive();
}
