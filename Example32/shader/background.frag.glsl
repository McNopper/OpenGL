#version 410 core

#define GLUS_PI 3.1415926535897932384626433832795

uniform sampler2D u_panoramaTexture;

in vec3 v_ray;

out vec4 fragColor;

//
// Map from normalized ray to st-coordinate of the panorma texture.
//
// see http://gl.ict.usc.edu/Data/HighResProbes/ 
// Note: that conversion has to be adapted to st-coordinates and the range of s to [0..1] 
//
vec2 panorama(vec3 ray)
{
	// Note: Two arguments of atan is atan2
	return vec2(0.5 + 0.5*atan(ray.x, -ray.z)/GLUS_PI, 1.0 - acos(ray.y)/GLUS_PI);
}

void main(void)
{
	vec3 ray = normalize(v_ray);

	fragColor = texture(u_panoramaTexture, panorama(ray));
}
