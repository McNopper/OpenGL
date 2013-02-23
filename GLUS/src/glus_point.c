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

GLUSvoid GLUSAPIENTRY glusPoint4Copyf(GLUSfloat result[4], const GLUSfloat point[4])
{
    result[0] = point[0];
    result[1] = point[1];
    result[2] = point[2];
    result[3] = point[3];
}

GLUSvoid GLUSAPIENTRY glusPoint3Copyf(GLUSfloat result[3], const GLUSfloat point[3])
{
    result[0] = point[0];
    result[1] = point[1];
    result[2] = point[2];
}

GLUSvoid GLUSAPIENTRY glusPoint4SubtractPoint4f(GLUSfloat result[3], const GLUSfloat point0[4], const GLUSfloat point1[4])
{
    result[0] = point0[0] - point1[0];
    result[1] = point0[1] - point1[1];
    result[2] = point0[2] - point1[2];
}

GLUSvoid GLUSAPIENTRY glusPoint3SubtractPoint3f(GLUSfloat result[2], const GLUSfloat point0[3], const GLUSfloat point1[3])
{
    result[0] = point0[0] - point1[0];
    result[1] = point0[1] - point1[1];
}

GLUSvoid GLUSAPIENTRY glusPoint4AddVector3f(GLUSfloat result[4], const GLUSfloat point[4], const GLUSfloat vector[3])
{
    result[0] = point[0] + vector[0];
    result[1] = point[1] + vector[1];
    result[2] = point[2] + vector[2];
    result[3] = point[3];
}

GLUSvoid GLUSAPIENTRY glusPoint3AddVector2f(GLUSfloat result[3], const GLUSfloat point[3], const GLUSfloat vector[2])
{
    result[0] = point[0] + vector[0];
    result[1] = point[1] + vector[1];
    result[2] = point[2];
}

GLUSvoid GLUSAPIENTRY glusPoint4SubtractVector3f(GLUSfloat result[4], const GLUSfloat point[4], const GLUSfloat vector[3])
{
    result[0] = point[0] - vector[0];
    result[1] = point[1] - vector[1];
    result[2] = point[2] - vector[2];
    result[3] = point[3];
}

GLUSvoid GLUSAPIENTRY glusPoint3SubtractVector2f(GLUSfloat result[3], const GLUSfloat point[3], const GLUSfloat vector[2])
{
    result[0] = point[0] - vector[0];
    result[1] = point[1] - vector[1];
    result[2] = point[2];
}

GLUSvoid GLUSAPIENTRY glusPoint4GetQuaternionf(GLUSfloat result[4], const GLUSfloat point[4])
{
    result[0] = point[0];
    result[1] = point[1];
    result[2] = point[2];
    result[3] = 0.0f;
}

GLUSvoid GLUSAPIENTRY glusPoint4GetVector3f(GLUSfloat result[3], const GLUSfloat point[4])
{
    result[0] = point[0];
    result[1] = point[1];
    result[2] = point[2];
}

GLUSvoid GLUSAPIENTRY glusPoint3GetVector2f(GLUSfloat result[2], const GLUSfloat point[3])
{
    result[0] = point[0];
    result[1] = point[1];
}

GLUSfloat GLUSAPIENTRY glusPoint4Distancef(const GLUSfloat point0[4], const GLUSfloat point1[4])
{
	GLUSfloat vector[3];

	glusPoint4SubtractPoint4f(vector, point0, point1);

	return glusVector3Lengthf(vector);
}

GLUSfloat GLUSAPIENTRY glusPoint3Distancef(const GLUSfloat point0[3], const GLUSfloat point1[3])
{
	GLUSfloat vector[2];

	glusPoint3SubtractPoint3f(vector, point0, point1);

	return glusVector2Lengthf(vector);
}
