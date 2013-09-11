#version 150

out vec2 v_texCoord;

void main(void)
{
	vec4 vertices[4] = {{-1.0, -1.0, 0.0, 1.0},
						{1.0, -1.0, 0.0, 1.0},
						{-1.0, 1.0, 0.0, 1.0},
						{1.0, 1.0, 0.0, 1.0}};

	vec2 texCoord[4] = {{0.0, 0.0},
						{1.0, 0.0},
						{0.0, 1.0},
						{1.0, 1.0}};

	v_texCoord = texCoord[gl_VertexID];

	gl_Position = vertices[gl_VertexID];
}
