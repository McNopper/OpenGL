#version 150

uniform mat4 u_modelViewProjectionMatrix;

uniform sampler2D u_positionTexture;

in vec2 a_vertex;

out float v_alpha;

void main(void)
{
	// x, y and z do store the position
	vec4 particle = vec4(texture(u_positionTexture, a_vertex).xyz, 1.0);

	// w contains the lifetime - which used for the alpha value
	v_alpha = texture(u_positionTexture, a_vertex).w;

	gl_PointSize = 32.0;
	
	gl_Position = u_modelViewProjectionMatrix*particle;
}
