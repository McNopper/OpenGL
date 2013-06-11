#version 410 core

layout(vertices = 3) out;

uniform mat4 u_projectionMatrix;
uniform mat4 u_viewMatrix;
uniform mat4 u_modelMatrix;

uniform float u_width;
uniform float u_height;

uniform float u_screenDistance;

uniform int u_doTessellate;

in vec3 v_c_tangent[];
in vec3 v_c_bitangent[];
in vec3 v_c_normal[];
in vec2 v_c_texCoord[];

out vec3 v_e_tangent[];
out vec3 v_e_bitangent[];
out vec3 v_e_normal[];
out vec2 v_e_texCoord[];

void main(void)
{
	// Dynamic tessellation level depending on screen space
	if (gl_InvocationID == 0)
	{
		if (u_doTessellate != 0)
		{
			vec2 screenPos[3];
			
			for (int i = 0; i < 3; i++)
			{
				vec4 position = u_projectionMatrix * u_viewMatrix * u_modelMatrix * gl_in[i].gl_Position;
				
				if (position.w != 0.0)
				{
					position /= position.w; 
				}
				
				screenPos[i].x = (position.x + 1.0) * 0.5 * u_width;
				screenPos[i].y = (position.y + 1.0) * 0.5 * u_height;  
			}
		
			// Tessellate depending how big the distance is on the screen
			gl_TessLevelOuter[0] = max(distance(screenPos[1], screenPos[2]) / u_screenDistance, 1.0);
			gl_TessLevelOuter[1] = max(distance(screenPos[2], screenPos[0]) / u_screenDistance, 1.0);
			gl_TessLevelOuter[2] = max(distance(screenPos[0], screenPos[1]) / u_screenDistance, 1.0);
		
			gl_TessLevelInner[0] = (gl_TessLevelOuter[0] + gl_TessLevelOuter[1] + gl_TessLevelOuter[2]) / 3.0;
		}
		else
		{
			gl_TessLevelOuter[0] = 1.0;
			gl_TessLevelOuter[1] = 1.0;
			gl_TessLevelOuter[2] = 1.0;
		
			gl_TessLevelInner[0] = 1.0;
		}
	}

	v_e_tangent[gl_InvocationID] = v_c_tangent[gl_InvocationID];
	v_e_bitangent[gl_InvocationID] = v_c_bitangent[gl_InvocationID];
	v_e_normal[gl_InvocationID] = v_c_normal[gl_InvocationID];
	v_e_texCoord[gl_InvocationID] = v_c_texCoord[gl_InvocationID];

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
