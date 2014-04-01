#version 410 core

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;

uniform vec4 u_eye;

in vec4 a_vertex;
in vec3 a_normal;

out vec3 v_eye;

out vec3 v_tangent;
out vec3 v_bitangent;
out vec3 v_normal;

void calculateBasis(out vec3 tangent, out vec3 bitangent, in vec3 normal)
{
	bitangent = vec3(0.0, 1.0, 0.0);

	float normalDotUp = dot(normal, bitangent);

	if (normalDotUp == 1.0)
	{
		bitangent = vec3(0.0, 0.0, -1.0);
	}
	else if (normalDotUp == -1.0)
	{
		bitangent = vec3(0.0, 0.0, 1.0);
	}
	
	tangent = cross(bitangent, normal);	
	bitangent = cross(normal, tangent);
} 

void main(void)
{
	v_normal = u_normalMatrix * a_normal;
	
	calculateBasis(v_tangent, v_bitangent, v_normal);
	
	vec4 vertex = u_modelMatrix * a_vertex;
	
	v_eye = (u_eye - vertex).xyz; 

	gl_Position = u_viewProjectionMatrix * vertex;
}
