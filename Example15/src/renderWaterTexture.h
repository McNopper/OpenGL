/**
 * OpenGL 3 - Example 15
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include "GL/glus.h"

typedef struct _WaveParameters
{
	GLfloat speed;
	GLfloat amplitude;
	GLfloat wavelength;
	GLfloat steepness;
} WaveParameters;

typedef struct _WaveDirections
{
	GLfloat x;
	GLfloat z;
} WaveDirections;

GLUSuint initWaterTexture(GLUSfloat waterPlaneLength);

GLUSvoid reshapeWaterTexture(GLUSint width, GLUSint height);

GLUSboolean renderWaterTexture(GLUSfloat time);

GLUSvoid terminateWaterTexture(GLUSvoid);
