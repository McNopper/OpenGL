#version 410 core

#define OBJECTS_COLUMNS 16

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;
uniform mat3 u_normalMatrix;

in vec4 a_vertex;
in vec3 a_normal;
in vec2 a_texCoord;

out vec4 v_position;
out vec3 v_normal;
out vec2 v_texCoord;

void main()
{
	mat4 offsetMatrix = mat4(1.0);
	
	float column = float(gl_InstanceID / OBJECTS_COLUMNS);
	float row = float(gl_InstanceID % OBJECTS_COLUMNS);
	
	offsetMatrix[3][0] = column;
	offsetMatrix[3][2] = -row;

	v_position = u_viewMatrix * offsetMatrix * u_modelMatrix * a_vertex;

	// Offset matrix has no rotations - so no need for the normal
	v_normal = mat3(u_viewMatrix) * u_normalMatrix * a_normal;
		
	v_texCoord = a_texCoord;
		
	gl_Position = u_projectionMatrix * v_position;
}    