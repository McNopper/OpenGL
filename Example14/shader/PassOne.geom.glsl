#version 410 core

layout(points, invocations = 1) in;
layout(points, max_vertices = 64) out;

uniform float u_halfDetailStep;

uniform uint u_detailLevel;

uniform float u_fovRadius;

uniform vec4 u_positionTextureSpace;
uniform vec3 u_leftNormalTextureSpace;
uniform vec3 u_rightNormalTextureSpace;
uniform vec3 u_backNormalTextureSpace;

out vec2 v_vertex;

bool isOutside(vec4 point, vec4 viewPoint, float step)
{
	float bias = 0.1;

	vec3 viewVector = vec3(point - viewPoint);

	float boundingRadius = sqrt(step*step+step*step);

	// Outside field of view
	if (length(viewVector) - boundingRadius > u_fovRadius)
	{
		return true;
	}

	// Behind the viewer
	if (dot(viewVector, u_backNormalTextureSpace) > boundingRadius + bias)
	{
		return true;
	}

	// Left side of field of view
	if (dot(viewVector, u_leftNormalTextureSpace) > boundingRadius + bias)
	{
		return true;
	}

	// Right side of field of view 
	if (dot(viewVector, u_rightNormalTextureSpace) > boundingRadius + bias)
	{
		return true;
	}
	
	return false;
}

void main(void)
{
	if (isOutside(gl_in[0].gl_Position, u_positionTextureSpace, u_halfDetailStep))	
	{
		return;
	}

	uint steps = uint(pow(2.0, float(u_detailLevel)));

	float finalDetailStep = u_halfDetailStep * 2.0 / float(steps);
	
	float halfFinalDetailStep = finalDetailStep / 2.0;

	vec4 centerPoint;

	float xFloat;
	float zFloat;

	for (uint z = 0; z < steps; z++)
	{
		zFloat = float(z);
	
		for (uint x = 0; x < steps; x++)
		{	
			xFloat = float(x);
		
			centerPoint = vec4(gl_in[0].gl_Position.x + xFloat * finalDetailStep - u_halfDetailStep + halfFinalDetailStep, 0.0, gl_in[0].gl_Position.z + zFloat * finalDetailStep - u_halfDetailStep + halfFinalDetailStep, 1.0);
		
			if (isOutside(centerPoint, u_positionTextureSpace, halfFinalDetailStep))
			{
				continue;
			}
		
			v_vertex = vec2(gl_in[0].gl_Position.x + xFloat * finalDetailStep - u_halfDetailStep, gl_in[0].gl_Position.z + zFloat * finalDetailStep - u_halfDetailStep);
			EmitVertex();
			EndPrimitive();

			v_vertex = vec2(gl_in[0].gl_Position.x + (xFloat + 1.0) * finalDetailStep - u_halfDetailStep, gl_in[0].gl_Position.z + zFloat * finalDetailStep - u_halfDetailStep);
			EmitVertex();
			EndPrimitive();

			v_vertex = vec2(gl_in[0].gl_Position.x + (xFloat + 1.0) * finalDetailStep - u_halfDetailStep, gl_in[0].gl_Position.z + (zFloat + 1.0) * finalDetailStep - u_halfDetailStep);
			EmitVertex();
			EndPrimitive();

			v_vertex = vec2(gl_in[0].gl_Position.x + xFloat * finalDetailStep - u_halfDetailStep, gl_in[0].gl_Position.z + (zFloat + 1.0) * finalDetailStep - u_halfDetailStep);
			EmitVertex();
			EndPrimitive();
		}
	}
}
