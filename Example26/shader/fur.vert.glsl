#version 150

in vec4 a_vertex;
in vec3 a_normal;
in vec2 a_texCoord;

out vec3 v_g_normal;
out vec2 v_g_texCoord;

void main()
{
	v_g_normal = a_normal;
		
	v_g_texCoord = a_texCoord;
		
	gl_Position = a_vertex;
}    