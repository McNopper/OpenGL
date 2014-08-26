/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) since 2010 Norbert Nopper
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GL/glus.h"

GLUSboolean GLUSAPIENTRY glusRaytracePerspectivef(GLUSfloat* directionBuffer, const GLUSubyte padding, const GLUSfloat fovy, const GLUSint width, const GLUSint height)
{
	GLUSint i, k;

	GLUSfloat aspect;

	GLUSfloat yExtend;
	GLUSfloat xExtend;

	GLUSfloat xStep;
	GLUSfloat yStep;

	if (!directionBuffer || width <= 0 || height <= 0)
	{
		return GLUS_FALSE;
	}

	aspect = (GLUSfloat)width / (GLUSfloat)height;

	yExtend = tanf(glusMathDegToRadf(fovy * 0.5f));
	xExtend = yExtend * aspect;

	xStep = xExtend / ((GLUSfloat)(width) * 0.5f);
	yStep = yExtend / ((GLUSfloat)(height) * 0.5f);

	for (i = 0; i < width * height; i++)
	{
		directionBuffer[i * (3 + padding) + 0] = -xExtend + xStep * 0.5f + xStep * (GLUSfloat)(i % width);
		directionBuffer[i * (3 + padding) + 1] = -yExtend + yStep * 0.5f + yStep * (GLUSfloat)(i / width);
		directionBuffer[i * (3 + padding) + 2] = -1.0f;

		for (k = 0; k < padding; k++)
		{
			directionBuffer[i * (3 + padding) + 3 + k] = 0.0f;
		}

		glusVector3Normalizef(&directionBuffer[i * (3 + padding)]);
	}

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusRaytraceLookAtf(GLUSfloat* positionBuffer, GLUSfloat* directionBuffer, const GLUSfloat* originDirectionBuffer, const GLUSubyte padding, const GLUSint width, const GLUSint height, const GLUSfloat eyeX, const GLUSfloat eyeY, const GLUSfloat eyeZ, const GLUSfloat centerX, const GLUSfloat centerY, const GLUSfloat centerZ, const GLUSfloat upX, const GLUSfloat upY, const GLUSfloat upZ)
{
	GLUSfloat forward[3], side[3], up[3];
	GLUSfloat rotation[9];
	GLUSint i, k;

	forward[0] = centerX - eyeX;
	forward[1] = centerY - eyeY;
	forward[2] = centerZ - eyeZ;

	glusVector3Normalizef(forward);

	up[0] = upX;
	up[1] = upY;
	up[2] = upZ;

	glusVector3Crossf(side, forward, up);
	glusVector3Normalizef(side);

	glusVector3Crossf(up, side, forward);

	rotation[0] = side[0];
	rotation[1] = side[1];
	rotation[2] = side[2];
	rotation[3] = up[0];
	rotation[4] = up[1];
	rotation[5] = up[2];
	rotation[6] = -forward[0];
	rotation[7] = -forward[1];
	rotation[8] = -forward[2];

	for (i = 0; i < width * height; i++)
	{
		if (positionBuffer)
		{
			positionBuffer[i * 4 + 0] = eyeX;
			positionBuffer[i * 4 + 1] = eyeY;
			positionBuffer[i * 4 + 2] = eyeZ;
			positionBuffer[i * 4 + 3] = 1.0f;
		}

		if (directionBuffer && originDirectionBuffer)
		{
			glusMatrix3x3MultiplyVector3f(&directionBuffer[i * (3 + padding)], rotation, &originDirectionBuffer[i * (3 + padding)]);

			for (k = 0; k < padding; k++)
			{
				directionBuffer[i * (3 + padding) + 3 + k] = originDirectionBuffer[i * (3 + padding) + 3 + k];
			}
		}
	}
}
