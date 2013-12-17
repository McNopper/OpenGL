#version 410 core

#define POINT_LIGHT_COUNT 16

#define POINT_LIGHT_COUNT_HALF (POINT_LIGHT_COUNT / 2)

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;
uniform mat4 u_positionMatrix;

in vec4 a_vertex;

flat out vec4 v_lightPosition;
flat out vec4 v_lightColor;
out vec4 v_lightBorderPosition;

void main()
{
	mat4 offsetMatrix = mat4(1.0);
	
	// 16 moving lights lights. Placed between rows and columns. Position and color is generated depending
	// on the instance ID. Alternative would be a uniform buffer.
	
	// First eight lights move along the x axis ...
	float column = float(gl_InstanceID / POINT_LIGHT_COUNT_HALF);
	float row = float(gl_InstanceID % POINT_LIGHT_COUNT_HALF);
	float x = u_positionMatrix[gl_InstanceID / 4 ][gl_InstanceID % 4];
	float z = 0.0;
	
	// ... last eight lights move along z axis.
	if (gl_InstanceID >= 8)
	{
		column = row;
		row = 0;
		
		float temp = x;
		x = z;
		z = temp;
	}
	
	offsetMatrix[3][0] = column * 2.0 + x;
	offsetMatrix[3][1] = 0.5;
	offsetMatrix[3][2] = -row * 2.0 - z;
	
	v_lightPosition = u_viewMatrix * offsetMatrix * u_modelMatrix * vec4(0.0, 0.0, 0.0, 1.0);
	
	v_lightBorderPosition = u_projectionMatrix * u_viewMatrix * offsetMatrix * u_modelMatrix * a_vertex;

	gl_Position = v_lightBorderPosition;

	// Generate different colors
	switch (gl_InstanceID)
	{
		case 0:
		case 8:
			v_lightColor = vec4(1.0, 1.0, 1.0, 1.0);		
		break;
		case 1:
		case 9:
			v_lightColor = vec4(1.0, 1.0, 0.0, 1.0);		
		break;
		case 2:
		case 10:
			v_lightColor = vec4(1.0, 0.0, 1.0, 1.0);		
		break;
		case 3:
		case 11:		
			v_lightColor = vec4(0.0, 1.0, 1.0, 1.0);		
		break;
		case 4:
		case 12:
			v_lightColor = vec4(1.0, 1.0, 1.0, 1.0);		
		break;
		case 5:
		case 13:
			v_lightColor = vec4(1.0, 0.0, 0.0, 1.0);		
		break;
		case 6:
		case 14:
			v_lightColor = vec4(0.0, 1.0, 0.0, 1.0);		
		break;
		case 7:
		case 15:
			v_lightColor = vec4(0.0, 0.0, 1.0, 1.0);		
		break;
		default:
			v_lightColor = vec4(0.0, 0.0, 0.0, 1.0);
	} 
}    