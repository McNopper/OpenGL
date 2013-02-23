#version 150

// Indices of refraction
const float Air = 1.0;
const float Glass = 1.51714;

// Air to glass ratio of the indices of refraction (Eta)
const float Eta = Air / Glass;
 
// see http://en.wikipedia.org/wiki/Refractive_index Reflectivity
const float R0 = ((Air - Glass) * (Air - Glass)) / ((Air + Glass) * (Air + Glass));

uniform mat4 u_viewProjectionMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;
uniform vec4 u_camera;

in vec4 a_vertex;
in vec3 a_normal;

out vec3 v_reflection;
out vec3 v_refraction;
out float v_fresnel;

void main(void)
{
	// We calculate in world space.
	
	vec4 vertex = u_modelMatrix*a_vertex;
	
	vec3 incident = normalize(vec3(vertex-u_camera));

	// Assume incoming normal is normalized.
	vec3 normal = u_normalMatrix*a_normal;
	
	v_refraction = refract(incident, normal, Eta);
	v_reflection = reflect(incident, normal);
			
	// see http://en.wikipedia.org/wiki/Schlick%27s_approximation
	v_fresnel = R0 + (1.0 - R0) * pow((1.0 - dot(-incident, normal)), 5.0);
		
	gl_Position = u_viewProjectionMatrix*vertex;
}
