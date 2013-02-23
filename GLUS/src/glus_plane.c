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

GLUSvoid GLUSAPIENTRY glusPlaneCopyf(GLUSfloat result[4], const GLUSfloat plane[4])
{
    result[0] = plane[0];
    result[1] = plane[1];
    result[2] = plane[2];
    result[3] = plane[3];
}

GLUSvoid GLUSAPIENTRY glusPlaneCreatef(GLUSfloat result[4], const GLUSfloat point[4], const GLUSfloat normal[3])
{
    GLUSfloat normalized[3];

    glusVector3Copyf(normalized, normal);
    glusVector3Normalizef(normalized);

    result[0] = normalized[0];
    result[1] = normalized[1];
    result[2] = normalized[2];

    // D = -N*P
    result[3] = -glusVector3Dotf(normalized, point);
}

GLUSfloat GLUSAPIENTRY glusPlaneDistancePoint4f(const GLUSfloat plane[4], const GLUSfloat point[4])
{
	return glusVector3Dotf(plane, point) + plane[3];
}
