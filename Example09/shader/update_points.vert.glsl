#version 150

uniform sampler2D u_positionTexture;

uniform float u_positionTextureWidth;

uniform float u_time;

in vec2 a_vertex;

out vec4 v_particle;

// Pseudo random function
float rand(vec2 seed)
{
	return fract(sin(dot(seed.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

void main(void)
{
	float position_speed = 0.2*u_time;
	float lifetime_speed = 0.04*u_time;

	// Get particle data from last texture
	v_particle = texture(u_positionTexture, a_vertex);

	// Each particle has the same translation vector. Pseudo random function is used to randomly distribute the particles over the screen
	v_particle.x += (rand(a_vertex*123.456)*2.0-1.0)*position_speed;
	v_particle.y += (rand(a_vertex*234.567)*2.0-1.0)*position_speed;
	v_particle.z += (rand(a_vertex*345.678)*2.0-1.0)*position_speed;
	
	// Decrease the lifetime
	v_particle.w -= lifetime_speed;

	// Particle died, create new one
	if (v_particle.w < 0.0)
	{
		v_particle.x = 0.0;
		v_particle.y = 0.0;
		v_particle.z = 0.0;
		v_particle.w = clamp(1.0 - rand(a_vertex*456.789), 0.0, 1.0);
	}
	
	// Only write to one texel
	gl_PointSize = 1.0;
	
	// Translate to NDC, which is between -1.0 to +1.0
	gl_Position = vec4(-vec2(1.0-1.0/u_positionTextureWidth, 1.0-1.0/u_positionTextureWidth) + a_vertex*2.0, 0.0, 1.0);
}
