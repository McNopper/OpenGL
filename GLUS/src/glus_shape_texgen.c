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

GLUSboolean GLUSAPIENTRY glusTexGenf(GLUSshape* shape, const GLUSfloat sXScale, const GLUSfloat sZScale, const GLUSfloat tYScale, const GLUSfloat tZScale, const GLUSfloat sOffset, const GLUSfloat tOffset)
{
	GLUSuint i, k;

	GLUSfloat v[3];

	if (!shape)
	{
		return GLUS_FALSE;
	}

	if (shape->texCoords)
	{
		free(shape->texCoords);

		shape->texCoords = 0;
	}

	shape->texCoords = (GLUSfloat*)malloc(2 * shape->numberVertices * sizeof(GLUSfloat));

	if (!shape->texCoords)
	{
		return GLUS_FALSE;
	}

	for (i = 0; i < shape->numberVertices; i++)
	{
		for (k = 0; k < 3; k++)
		{
			v[k] = shape->vertices[4 * i + k];
		}

		shape->texCoords[2 * i + 0] = v[0] * sXScale + v[2] * sZScale + sOffset;
		shape->texCoords[2 * i + 1] = v[1] * tYScale + v[2] * tZScale + tOffset;
	}

	return GLUS_TRUE;
}
