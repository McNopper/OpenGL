#version 410 core

layout(triangles) in;

layout(triangle_strip, max_vertices = 3) out;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;

uniform sampler2D u_normalTexture;

uniform float u_displacementScale;

in vec3 v_g_tangent[3];
in vec3 v_g_bitangent[3];
in vec3 v_g_normal[3];
in vec2 v_g_texCoord[3];

out vec3 v_tangent;
out vec3 v_bitangent;
out vec3 v_normal;
out vec2 v_texCoord;

void main(void)
{
	for(int i = 0; i < gl_in.length(); ++i)
	{
		v_tangent = v_g_tangent[i];
		v_bitangent = v_g_bitangent[i];
		v_normal = v_g_normal[i];
		v_texCoord = v_g_texCoord[i];
		
		float displacementOffset = texture(u_normalTexture, v_texCoord).a * u_displacementScale;
		vec4 displacement = vec4(normalize(v_g_normal[i]) * displacementOffset, 0.0);
	
		gl_Position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * (gl_in[i].gl_Position + displacement);

		EmitVertex();
	}
	
	EndPrimitive();
}
