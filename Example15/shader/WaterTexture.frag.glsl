#version 150

#define NUMBERWAVES 4

const float Eta = 0.15; // Water
const float PI = 3.141592654;
const float G = 9.81;

uniform float u_waterPlaneLength;

uniform float u_passedTime;

uniform vec4 u_waveParameters[NUMBERWAVES];
uniform vec2 u_waveDirections[NUMBERWAVES];

in vec2 v_texCoord;

out vec4 fragColor;

vec3 normalToTexture(vec3 orgNormal)
{
	return vec3(clamp(orgNormal.x*0.5 + 0.5, 0.0, 1.0), clamp(orgNormal.y*0.5 + 0.5, 0.0, 1.0), clamp(orgNormal.z*0.5 + 0.5, 0.0, 1.0));
}

void main(void)
{
	vec3 vertex = vec3(v_texCoord.s*u_waterPlaneLength-u_waterPlaneLength/2.0 + 0.5, 0.0, -v_texCoord.t*u_waterPlaneLength+u_waterPlaneLength/2.0 + 0.5);

	vec4 finalVertex;

	finalVertex.x = vertex.x;
	finalVertex.y = vertex.y;
	finalVertex.z = vertex.z;
	finalVertex.w = 1.0;

	vec3 finalNormal;
	
	finalNormal.x = 0.0;
	finalNormal.y = 0.0;
	finalNormal.z = 0.0;

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
		float alpha = frequency*dot(direction, vertex.xz)+phase*u_passedTime;
		
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
				
		finalNormal.x += direction.x * wavelength * amplitude * cos(alpha);
		finalNormal.y += steepness * wavelength * amplitude * sin(alpha);
		finalNormal.z += direction.y * wavelength * amplitude * cos(alpha);
	}

	finalNormal.x = -finalNormal.x;
	finalNormal.y = 1.0 - finalNormal.y;
	finalNormal.z = -finalNormal.z;
	finalNormal = normalize(finalNormal);

	fragColor = vec4(normalToTexture(finalNormal), 1.0);
}
