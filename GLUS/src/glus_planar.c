/*
 * GLUS - OpenGL 3 and 4 Utilities. Copyright (C) 2010 - 2013 Norbert Nopper
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

GLUSvoid GLUSAPIENTRY glusPlanarShadowPointLightf(GLUSfloat matrix[16], const GLUSfloat shadowPlane[4], const GLUSfloat lightPoint[4])
{
	GLUSfloat nDotL = glusVector3Dotf(shadowPlane, lightPoint);

	matrix[0] = nDotL + shadowPlane[3] - lightPoint[0] * shadowPlane[0];
	matrix[1] = -lightPoint[1] * shadowPlane[0];
	matrix[2] = -lightPoint[2] * shadowPlane[0];
	matrix[3] = -shadowPlane[0];

	matrix[4] = -lightPoint[0] * shadowPlane[1];
	matrix[5] = nDotL + shadowPlane[3] - lightPoint[1] * shadowPlane[1];
	matrix[6] = -lightPoint[2] * shadowPlane[1];
	matrix[7] = -shadowPlane[1];

	matrix[8] = -lightPoint[0] * shadowPlane[2];
	matrix[9] = -lightPoint[1] * shadowPlane[2];
	matrix[10] = nDotL + shadowPlane[3] - lightPoint[2] * shadowPlane[2];
	matrix[11] = -shadowPlane[2];

	matrix[12] = -lightPoint[0] * shadowPlane[3];
	matrix[13] = -lightPoint[1] * shadowPlane[3];
	matrix[14] = -lightPoint[2] * shadowPlane[3];
	matrix[15] = nDotL;
}

GLUSvoid GLUSAPIENTRY glusPlanarShadowDirectionalLightf(GLUSfloat matrix[16], const GLUSfloat shadowPlane[4], const GLUSfloat lightDirection[3])
{
	GLUSfloat lightDirectionNormalized[3];

	GLUSfloat nDotL;

	glusVector3Copyf(lightDirectionNormalized, lightDirection);
	glusVector3Normalizef(lightDirectionNormalized);

	nDotL = glusVector3Dotf(shadowPlane, lightDirectionNormalized);

	matrix[0] = nDotL - lightDirectionNormalized[0] * shadowPlane[0];
	matrix[1] = -lightDirectionNormalized[1] * shadowPlane[0];
	matrix[2] = -lightDirectionNormalized[2] * shadowPlane[0];
	matrix[3] = 0.0f;

	matrix[4] = -lightDirectionNormalized[0] * shadowPlane[1];
	matrix[5] = nDotL - lightDirectionNormalized[1] * shadowPlane[1];
	matrix[6] = -lightDirectionNormalized[2] * shadowPlane[1];
	matrix[7] = 0.0f;

	matrix[8] = -lightDirectionNormalized[0] * shadowPlane[2];
	matrix[9] = -lightDirectionNormalized[1] * shadowPlane[2];
	matrix[10] = nDotL - lightDirectionNormalized[2] * shadowPlane[2];
	matrix[11] = 0.0f;

	matrix[12] = -lightDirectionNormalized[0] * shadowPlane[3];
	matrix[13] = -lightDirectionNormalized[1] * shadowPlane[3];
	matrix[14] = -lightDirectionNormalized[2] * shadowPlane[3];
	matrix[15] = nDotL;
}

GLUSvoid GLUSAPIENTRY glusPlanarReflectionf(GLUSfloat matrix[16], const GLUSfloat reflectionPlane[4])
{
	GLUSfloat pointOnPlane[4];
	GLUSfloat yUpNormal[3];
	GLUSfloat rotationAngle;
	GLUSfloat rotationAxis[3];

	glusPlaneGetPoint4f(pointOnPlane, reflectionPlane);

	yUpNormal[0] = 0.0f;
	yUpNormal[1] = 1.0f;
	yUpNormal[2] = 0.0f;

	rotationAngle = glusRadToDegf(acosf(glusVector3Dotf(reflectionPlane, yUpNormal)));

	if (rotationAngle != 0.0f)
	{
		glusVector3Crossf(rotationAxis, yUpNormal, reflectionPlane);

		glusVector3Normalizef(rotationAxis);
	}

	glusMatrix4x4Identityf(matrix);

	if (pointOnPlane[0] != 0.0f || pointOnPlane[1] != 0.0f || pointOnPlane[2] != 0.0f)
	{
		glusMatrix4x4Translatef(matrix, pointOnPlane[0], pointOnPlane[1], pointOnPlane[2]);
	}

	if (rotationAngle != 0.0f)
	{
		glusMatrix4x4Rotatef(matrix, rotationAngle, rotationAxis[0], rotationAxis[1], rotationAxis[2]);
	}

	glusMatrix4x4Scalef(matrix, 1.0f, -1.0f, 1.0f);

	if (rotationAngle != 0.0f)
	{
		glusMatrix4x4Rotatef(matrix, -rotationAngle, rotationAxis[0], rotationAxis[1], rotationAxis[2]);
	}

	if (pointOnPlane[0] != 0.0f || pointOnPlane[1] != 0.0f || pointOnPlane[2] != 0.0f)
	{
		glusMatrix4x4Translatef(matrix, -pointOnPlane[0], -pointOnPlane[1], -pointOnPlane[2]);
	}
}
