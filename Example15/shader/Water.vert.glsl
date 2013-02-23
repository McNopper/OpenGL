#version 150

#define NUMBERWAVES 4

const float PI = 3.141592654;
const float G = 9.81;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat3 u_inverseViewNormalMatrix;

uniform float u_passedTime;

uniform float u_waterPlaneLength;

uniform vec4 u_waveParameters[NUMBERWAVES];
uniform vec2 u_waveDirections[NUMBERWAVES];

in vec4 a_vertex;

out vec3 v_incident;

out vec3 v_bitangent;
out vec3 v_normal;
out vec3 v_tangent;

out vec2 v_texCoord;

void main(void)
{
	vec4 finalVertex;

	finalVertex.x = a_vertex.x;
	finalVertex.y = a_vertex.y;
	finalVertex.z = a_vertex.z;
	finalVertex.w = 1.0;

	vec3 finalBitangent;
	vec3 finalNormal;
	vec3 finalTangent;

	finalBitangent.x = 0.0;
	finalBitangent.y = 0.0;
	finalBitangent.z = 0.0;
	
	finalNormal.x = 0.0;
	finalNormal.y = 0.0;
	finalNormal.z = 0.0;

	finalTangent.x = 0.0;
	finalTangent.y = 0.0;
	finalTangent.z = 0.0;

	// see GPU Gems: Chapter 1. Effective Water Simulation from Physical Models
	for (int i = 0; i < NUMBERWAVES; i++)
	{
		vec2 direction = normalize(u_waveDirections[i]);
		float speed = u_waveParameters[i].x;
		float amplitude = u_waveParameters[i].y;
		float wavelength = u_waveParameters[i].z;
		float steepness = u_waveParameters[i].w;

		float frequency = sqrt(G*2.0*PI/wavelength);
		float phase = speed*frequency;
		float alpha = frequency*dot(direction, a_vertex.xz)+phase*u_passedTime;
		
		finalVertex.x += steepness*amplitude*direction.x*cos(alpha);
		finalVertex.y += amplitude*sin(alpha);
		finalVertex.z += steepness*amplitude*direction.y*cos(alpha);
	}

	for (int i = 0; i < NUMBERWAVES; i++)
	{
		vec2 direction = normalize(u_waveDirections[i]);
		float speed = u_waveParameters[i].x;
		float amplitude = u_waveParameters[i].y;
		float wavelength = u_waveParameters[i].z;
		float steepness = u_waveParameters[i].w;

		float frequency = sqrt(G*2.0*PI/wavelength);
		float phase = speed*frequency;
		float alpha = frequency * dot(direction, finalVertex.xz) + phase*u_passedTime;
				
		// x direction
		finalBitangent.x += steepness * direction.x*direction.x * wavelength * amplitude * sin(alpha);
		finalBitangent.y += direction.x * wavelength * amplitude * cos(alpha);	
		finalBitangent.z += steepness * direction.x*direction.y * wavelength * amplitude * sin(alpha);	
	
		// y direction
		finalNormal.x += direction.x * wavelength * amplitude * cos(alpha);
		finalNormal.y += steepness * wavelength * amplitude * sin(alpha);
		finalNormal.z += direction.y * wavelength * amplitude * cos(alpha);

		// z direction
		finalTangent.x += steepness * direction.x*direction.y * wavelength * amplitude * sin(alpha);
		finalTangent.y += direction.y * wavelength * amplitude * cos(alpha);	
		finalTangent.z += steepness * direction.y*direction.y * wavelength * amplitude * sin(alpha);	
	}

	finalTangent.x = -finalTangent.x;
	finalTangent.z = 1.0 - finalTangent.z;
	finalTangent = normalize(finalTangent);

	finalBitangent.x = 1.0 - finalBitangent.x;
	finalBitangent.z = -finalBitangent.z;
	finalBitangent = normalize(finalBitangent);

	finalNormal.x = -finalNormal.x;
	finalNormal.y = 1.0 - finalNormal.y;
	finalNormal.z = -finalNormal.z;
	finalNormal = normalize(finalNormal);
	
	v_bitangent = finalBitangent;
	v_normal = finalNormal;
	v_tangent = finalTangent;
	
	v_texCoord = vec2(clamp((finalVertex.x+u_waterPlaneLength*0.5-0.5)/u_waterPlaneLength, 0.0, 1.0), clamp((-finalVertex.z+u_waterPlaneLength*0.5+0.5)/u_waterPlaneLength, 0.0, 1.0));

	vec4 vertex = u_viewMatrix*finalVertex;
	
	// We caculate in world space.
	v_incident = u_inverseViewNormalMatrix * vec3(vertex);	
				
	gl_Position = u_projectionMatrix*vertex;
}
