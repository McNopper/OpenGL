#version 150

uniform mat4 u_projectionMatrix;
uniform mat4 u_modelViewMatrix;
uniform mat3 u_normalMatrix;

uniform vec3 u_lightDirection;

in vec4 a_vertex;
in vec3 a_normal;
in vec3 a_tangent;
in vec3 a_bitangent;
in vec2 a_texCoord;

out vec2 v_texCoord;
out vec3 v_tsLight;
out vec3 v_tsEye;

void main(void)
{
	v_texCoord = a_texCoord;

	// Bring tangent, bitangent and normal to camera space.
	vec3 tangent = u_normalMatrix*a_tangent;
	vec3 bitangent = u_normalMatrix*a_bitangent;
	vec3 normal = u_normalMatrix*a_normal;

	// The inverse brings the light from camera to texture space.
	// tx ty tz   lightDirectionx
	// bx by bz * lightDirectiony
	// nx ny nz   lightDirectionz
	v_tsLight.x = dot(tangent, u_lightDirection);
	v_tsLight.y = dot(bitangent, u_lightDirection);
	v_tsLight.z = dot(normal, u_lightDirection);
	
	// Bring vertex in camera space.
	vec4 vertex = u_modelViewMatrix*a_vertex;
	
	// Calculate eye vector, which is in camera space.
	vec3 eye = -vec3(vertex);
	
	// Like the light, this brings the eye vector to texture space.
	v_tsEye.x = dot(eye, tangent);
	v_tsEye.y = dot(eye, bitangent);
	v_tsEye.z = dot(eye, normal);

	gl_Position = u_projectionMatrix*vertex;
}
