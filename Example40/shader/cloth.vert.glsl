#version 430 core

layout(std430, binding=1) buffer Vertex {
    vec4 b_vertex[];
};

layout(std430, binding=2) buffer Normal {
    vec3 b_normal[];
    // Padding[]
};

uniform mat4 u_modelViewProjectionMatrix;
uniform mat3 u_normalMatrix;
uniform vec3 u_lightDirection;
uniform vec4 u_color;

out vec4 v_color;

void main(void)
{
	vec3 normal = u_normalMatrix * b_normal[gl_VertexID];

 	v_color = u_color * max(dot(normal, u_lightDirection), 0.0);

	gl_Position = u_modelViewProjectionMatrix * b_vertex[gl_VertexID];
}
