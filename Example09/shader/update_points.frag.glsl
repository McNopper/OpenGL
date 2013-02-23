#version 150

in vec4 v_particle;

out vec4 fragParticle;

void main(void)
{
	fragParticle = v_particle;
}
