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

GLUSvoid GLUSAPIENTRY glusPlanarShadowPointLightf(GLUSfloat result[16], const GLUSfloat shadowPlane[4], const GLUSfloat lightPoint[4])
{
	float nDotL = glusVector3Dotf(shadowPlane, lightPoint);

	result[0] = nDotL + shadowPlane[3] - lightPoint[0] * shadowPlane[0];
	result[1] = -lightPoint[1] * shadowPlane[0];
	result[2] = -lightPoint[2] * shadowPlane[0];
	result[3] = -shadowPlane[0];

	result[4] = -lightPoint[0] * shadowPlane[1];
	result[5] = nDotL + shadowPlane[3] - lightPoint[1] * shadowPlane[1];
	result[6] = -lightPoint[2] * shadowPlane[1];
	result[7] = -shadowPlane[1];

	result[8] = -lightPoint[0] * shadowPlane[2];
	result[9] = -lightPoint[1] * shadowPlane[2];
	result[10] = nDotL + shadowPlane[3] - lightPoint[2] * shadowPlane[2];
	result[11] = -shadowPlane[2];

	result[12] = -lightPoint[0] * shadowPlane[3];
	result[13] = -lightPoint[1] * shadowPlane[3];
	result[14] = -lightPoint[2] * shadowPlane[3];
	result[15] = nDotL;
}

GLUSvoid GLUSAPIENTRY glusPlanarShadowDirectionalLightf(GLUSfloat result[16], const GLUSfloat shadowPlane[4], const GLUSfloat lightDirection[3])
{
	float lightDirectionNormalized[3];

	float nDotL = glusVector3Dotf(shadowPlane, lightDirection);

	glusVector3Copyf(lightDirectionNormalized, lightDirection);
	glusVector3Normalizef(lightDirectionNormalized);

	result[0] = nDotL - lightDirectionNormalized[0] * shadowPlane[0];
	result[1] = -lightDirectionNormalized[1] * shadowPlane[0];
	result[2] = -lightDirectionNormalized[2] * shadowPlane[0];
	result[3] = shadowPlane[0] * lightDirectionNormalized[0];

	result[4] = -lightDirectionNormalized[0] * shadowPlane[1];
	result[5] = nDotL - lightDirectionNormalized[1] * shadowPlane[1];
	result[6] = -lightDirectionNormalized[2] * shadowPlane[1];
	result[7] = shadowPlane[1] * lightDirectionNormalized[1];

	result[8] = -lightDirectionNormalized[0] * shadowPlane[2];
	result[9] = -lightDirectionNormalized[1] * shadowPlane[2];
	result[10] = nDotL - lightDirectionNormalized[2] * shadowPlane[2];
	result[11] = shadowPlane[2] * lightDirectionNormalized[2];

	result[12] = -lightDirectionNormalized[0] * shadowPlane[3];
	result[13] = -lightDirectionNormalized[1] * shadowPlane[3];
	result[14] = -lightDirectionNormalized[2] * shadowPlane[3];
	result[15] = 0.0f;
}

GLUSvoid GLUSAPIENTRY glusPlanarReflectionf(GLUSfloat result[16], const GLUSfloat reflectionPlane[4])
{
	float yUpNormal[3];
	float rotationAngle;
	float rotationAxis[3];

	yUpNormal[0] = 0.0f;
	yUpNormal[1] = 1.0f;
	yUpNormal[2] = 0.0f;

	rotationAngle = glusRadToDegf(acosf(glusVector3Dotf(reflectionPlane, yUpNormal)));

	if (rotationAngle != 0.0f)
	{
		glusVector3Crossf(rotationAxis, yUpNormal, reflectionPlane);

		glusVector3Normalizef(rotationAxis);
	}

	glusMatrix4x4Identityf(result);

	if (reflectionPlane[3] != 0.0f)
	{
		glusMatrix4x4Translatef(result, 0.0f, reflectionPlane[3], 0.0f);
	}

	if (rotationAngle != 0.0f)
	{
		glusMatrix4x4Rotatef(result, rotationAngle, rotationAxis[0], rotationAxis[1], rotationAxis[2]);
	}

	glusMatrix4x4Scalef(result, 1.0f, -1.0f, 1.0f);

	if (rotationAngle != 0.0f)
	{
		glusMatrix4x4Rotatef(result, -rotationAngle, rotationAxis[0], rotationAxis[1], rotationAxis[2]);
	}

	if (reflectionPlane[3] != 0.0f)
	{
		glusMatrix4x4Translatef(result, 0.0f, -reflectionPlane[3], 0.0f);
	}
}
