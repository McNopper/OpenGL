#version 410 core

#define GLUS_PI 3.1415926535897932384626433832795

uniform sampler2D u_panoramaTexture;
uniform int u_face;

in vec2 v_texCoord;

out vec4 fragColor;

// see http://gl.ict.usc.edu/Data/HighResProbes/
vec2 panorama(vec3 ray)
{
	return vec2(0.5 + 0.5*atan(ray.x, -ray.z)/GLUS_PI, 1.0 - acos(ray.y)/GLUS_PI);
}

// Convert a cubemap face uv in [-1, 1]^2 to the corresponding 3D direction.
// Face mapping follows the OpenGL cubemap face orientation convention (Table 8.19).
vec3 cubemapDirection(int face, vec2 uv)
{
	switch (face)
	{
		case 0: return normalize(vec3( 1.0, -uv.y, -uv.x)); // +X
		case 1: return normalize(vec3(-1.0, -uv.y,  uv.x)); // -X
		case 2: return normalize(vec3( uv.x,  1.0,  uv.y)); // +Y
		case 3: return normalize(vec3( uv.x, -1.0, -uv.y)); // -Y
		case 4: return normalize(vec3( uv.x, -uv.y,  1.0)); // +Z
		case 5: return normalize(vec3(-uv.x, -uv.y, -1.0)); // -Z
	}
	return vec3(0.0);
}

void main(void)
{
	vec3 dir = cubemapDirection(u_face, v_texCoord);

	fragColor = vec4(texture(u_panoramaTexture, panorama(dir)).rgb, 1.0);
}
