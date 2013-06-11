#version 410 core

layout(vertices = 4) out;

uniform uint u_maxTessellationLevel;

uniform int u_quadrantStep;

uniform vec4 u_positionTextureSpace;

void main(void)
{
	float tessellationStep = pow(2.0, float(u_maxTessellationLevel));

	// Caclulate the quadrant / patch we are currently in
	int relativeQuadrantS = int((gl_in[0].gl_Position.x - u_positionTextureSpace.x) / tessellationStep);
	int relativeQuadrantT = int((gl_in[0].gl_Position.z - u_positionTextureSpace.z) / tessellationStep);

	int absRelativeQuadrantS = abs(relativeQuadrantS);
	int absRelativeQuadrantT = abs(relativeQuadrantT);

	// Depending on the Chebyshev Distance, we decide, if a border quadrant or not
	int chebyshevDistance = max(absRelativeQuadrantS, absRelativeQuadrantT);
	
	// (1) skip x elements and (2) outermost element and (3) on the left side
	bool leftBorder = ((absRelativeQuadrantS + 1) % u_quadrantStep == 0) && (absRelativeQuadrantS == chebyshevDistance) && (relativeQuadrantS <= 0);
	bool rightBorder = ((absRelativeQuadrantS + 1) % u_quadrantStep == 0) && (absRelativeQuadrantS == chebyshevDistance) && (relativeQuadrantS >= 0);

	bool bottomBorder = ((absRelativeQuadrantT + 1) % u_quadrantStep == 0) && (absRelativeQuadrantT == chebyshevDistance) && (relativeQuadrantT <= 0);
	bool topBorder = ((absRelativeQuadrantT + 1) % u_quadrantStep == 0) && (absRelativeQuadrantT == chebyshevDistance) && (relativeQuadrantT >= 0);

	// Tessellation level depends on distance, but calculate out the step 
	uint tessellationLevel = uint(chebyshevDistance / u_quadrantStep);

	// From the start, all do have the same tesseallation level

	uint decrease = min(u_maxTessellationLevel + 1, tessellationLevel);

	uint decreaseLeft = decrease; 
	uint decreaseRight = decrease;
	
	uint decreaseTop = decrease;
	uint decreaseBottom = decrease;
	
	uint decreaseInnner = decrease;

	// Depending on the border, change the decrease level. So we do have noartifacts when changing from a smaller to a larger patch and vice versa.

	decrease = min(u_maxTessellationLevel + 1, decrease + 1);

	if (leftBorder)
	{
		decreaseLeft = decrease;
	}	
	if (rightBorder)
	{
		decreaseRight = decrease;
	}
	
	if (bottomBorder)
	{
		decreaseBottom = decrease;
	}
	if (topBorder)
	{
		decreaseTop = decrease;
	}

	gl_TessLevelOuter[0] = pow(2.0, float(u_maxTessellationLevel + 1 - decreaseLeft));
	gl_TessLevelOuter[1] = pow(2.0, float(u_maxTessellationLevel + 1 - decreaseBottom));
	gl_TessLevelOuter[2] = pow(2.0, float(u_maxTessellationLevel + 1 - decreaseRight));
	gl_TessLevelOuter[3] = pow(2.0, float(u_maxTessellationLevel + 1 - decreaseTop));

	gl_TessLevelInner[0] = pow(2.0, float(u_maxTessellationLevel + 1 - decreaseInnner));
	gl_TessLevelInner[1] = pow(2.0, float(u_maxTessellationLevel + 1 - decreaseInnner));

	gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
}
