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

GLUSvoid GLUSAPIENTRY glusSphereCopyf(GLUSfloat resultCenter[4], GLUSfloat resultRadius, const GLUSfloat center[4], const GLUSfloat radius)
{
	resultCenter[0] = center[0];
	resultCenter[1] = center[1];
	resultCenter[2] = center[2];
	resultCenter[3] = center[3];

	resultRadius = radius;
}

GLUSfloat GLUSAPIENTRY glusSphereDistancePoint4f(const GLUSfloat center[4], const GLUSfloat radius, const GLUSfloat point[4])
{
	return glusPoint4Distancef(point, center) - radius;
}
