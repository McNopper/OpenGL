/*
 * GLUS - OpenGL 3 and 4 Utilities. Copyright (C) 2010 - 2014 Norbert Nopper
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

GLUSvoid GLUSAPIENTRY glusOrientedBoxCopyf(GLUSfloat resultCenter[4], GLUSfloat resultHalfExtend[3], GLUSfloat resultOrientation[3], const GLUSfloat center[4], const GLUSfloat halfExtend[3], const GLUSfloat orientation[3])
{
	resultCenter[0] = center[0];
	resultCenter[1] = center[1];
	resultCenter[2] = center[2];
	resultCenter[3] = center[3];

	resultHalfExtend[0] = halfExtend[0];
	resultHalfExtend[1] = halfExtend[1];
	resultHalfExtend[2] = halfExtend[2];
}

GLUSfloat GLUSAPIENTRY glusOrientedBoxDistancePoint4f(const GLUSfloat center[4], const GLUSfloat halfExtend[3], const GLUSfloat orientation[3], const GLUSfloat point[4])
{
	GLUSfloat matrix[9];
	GLUSfloat vector[3];

	GLUSfloat insideDistance;
	GLUSfloat outsideDistance;

	glusPoint4SubtractPoint4f(vector, point, center);

	glusMatrix3x3Identityf(matrix);
	glusMatrix3x3RotateRzRyRxf(matrix, -orientation[2], -orientation[1], -orientation[0]);
	glusMatrix3x3MultiplyVector3f(vector, matrix, vector);

	vector[0] = fabsf(vector[0]) - halfExtend[0];
	vector[1] = fabsf(vector[1]) - halfExtend[1];
	vector[2] = fabsf(vector[2]) - halfExtend[2];

	insideDistance = glusMinf(glusMaxf(vector[0], glusMaxf(vector[1], vector[2])), 0.0f);

	vector[0] = glusMaxf(vector[0], 0.0f);
	vector[1] = glusMaxf(vector[1], 0.0f);
	vector[2] = glusMaxf(vector[2], 0.0f);

	outsideDistance = glusVector3Lengthf(vector);

	return insideDistance + outsideDistance;
}
