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

static GLUSvoid glusInitLinef(GLUSline* line)
{
	if (!line)
	{
		return;
	}

	memset(line, 0, sizeof(GLUSline));

	line->mode = GL_LINE_LOOP;
}

static GLUSboolean glusCheckLinef(GLUSline* line)
{
	if (!line)
	{
		return GLUS_FALSE;
	}

	return line->vertices && line->numberIndices;
}

GLUSboolean GLUSAPIENTRY glusCreateLinef(GLUSline* line, const GLUSfloat point0[4], const GLUSfloat point1[4])
{
	if (!line)
	{
		return GLUS_FALSE;
	}
	glusInitLinef(line);

	line->numberVertices = 2;
	line->numberIndices = 2;

	line->vertices = (GLUSfloat*)malloc(4 * line->numberVertices * sizeof(GLUSfloat));
	line->indices = (GLUSuint*)malloc(line->numberIndices * sizeof(GLUSuint));

	line->mode = GL_LINES;

	if (!glusCheckLinef(line))
	{
		glusDestroyLinef(line);

		return GLUS_FALSE;
	}

	memcpy(line->vertices, point0, 4 * sizeof(GLUSfloat));
	memcpy(line->vertices + 4, point1, 4 * sizeof(GLUSfloat));

	line->indices[0] = 0;
	line->indices[1] = 1;

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusCreateSquaref(GLUSline* line, const GLUSfloat halfExtend)
{
	if (!line)
	{
		return GLUS_FALSE;
	}
	glusInitLinef(line);

	line->numberVertices = 4;
	line->numberIndices = 4;

	line->vertices = (GLUSfloat*)malloc(4 * line->numberVertices * sizeof(GLUSfloat));
	line->indices = (GLUSuint*)malloc(line->numberIndices * sizeof(GLUSuint));

	line->mode = GL_LINE_LOOP;

	if (!glusCheckLinef(line))
	{
		glusDestroyLinef(line);

		return GLUS_FALSE;
	}

	line->vertices[0] = halfExtend;
	line->vertices[1] = -halfExtend;
	line->vertices[2] = 0.0f;
	line->vertices[3] = 1.0f;

	line->vertices[4] = halfExtend;
	line->vertices[5] = halfExtend;
	line->vertices[6] = 0.0f;
	line->vertices[7] = 1.0f;

	line->vertices[8] = -halfExtend;
	line->vertices[9] = halfExtend;
	line->vertices[10] = 0.0f;
	line->vertices[11] = 1.0f;

	line->vertices[12] = -halfExtend;
	line->vertices[13] = -halfExtend;
	line->vertices[14] = 0.0f;
	line->vertices[15] = 1.0f;

	line->indices[0] = 0;
	line->indices[1] = 1;
	line->indices[2] = 2;
	line->indices[3] = 3;

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusCreateRectangularGridf(GLUSline* line, const GLUSfloat horizontalExtend, const GLUSfloat verticalExtend, const GLUSuint rows, const GLUSuint columns)
{
	GLUSuint i, offset;
	GLUSuint numberVertices = (rows + 1) * 2 + (columns + 1) * 2;
	GLUSuint numberIndices = numberVertices;
	float rowStep, columnStep;

	if (!line || rows < 1 || columns < 1 || numberVertices > GLUS_MAX_VERTICES || numberIndices > GLUS_MAX_INDICES)
	{
		return GLUS_FALSE;
	}
	glusInitLinef(line);

	line->numberVertices = numberVertices;
	line->numberIndices = numberIndices;

	line->vertices = (GLUSfloat*)malloc(4 * line->numberVertices * sizeof(GLUSfloat));
	line->indices = (GLUSuint*)malloc(line->numberIndices * sizeof(GLUSuint));

	line->mode = GL_LINES;

	if (!glusCheckLinef(line))
	{
		glusDestroyLinef(line);

		return GLUS_FALSE;
	}

	rowStep = verticalExtend / (float)rows;
	for (i = 0; i <= rows; i++)
	{
		line->vertices[0 + i * 4 * 2] = -horizontalExtend * 0.5f;
		line->vertices[1 + i * 4 * 2] = -verticalExtend * 0.5f + rowStep * (float)i;
		line->vertices[2 + i * 4 * 2] = 0.0f;
		line->vertices[3 + i * 4 * 2] = 1.0f;

		line->indices[i * 2] = i * 2;

		//

		line->vertices[0 + i * 4 * 2 + 4] = horizontalExtend * 0.5f;
		line->vertices[1 + i * 4 * 2 + 4] = -verticalExtend * 0.5f + rowStep * (float)i;
		line->vertices[2 + i * 4 * 2 + 4] = 0.0f;
		line->vertices[3 + i * 4 * 2 + 4] = 1.0f;

		line->indices[i * 2 + 1] = i * 2 + 1;
	}

	offset = (rows + 1) * 2;

	columnStep = horizontalExtend / (float)columns;
	for (i = 0; i <= columns; i++)
	{
		line->vertices[0 + i * 4 * 2 + offset * 4] = -horizontalExtend * 0.5f + columnStep * (float)i;
		line->vertices[1 + i * 4 * 2 + offset * 4] = -verticalExtend * 0.5f;
		line->vertices[2 + i * 4 * 2 + offset * 4] = 0.0f;
		line->vertices[3 + i * 4 * 2 + offset * 4] = 1.0f;

		line->indices[i * 2 + offset] = i * 2 + offset;

		//

		line->vertices[0 + i * 4 * 2 + 4 + offset * 4] = -horizontalExtend * 0.5f + columnStep * (float)i;
		line->vertices[1 + i * 4 * 2 + 4 + offset * 4] = verticalExtend * 0.5f;
		line->vertices[2 + i * 4 * 2 + 4 + offset * 4] = 0.0f;
		line->vertices[3 + i * 4 * 2 + 4 + offset * 4] = 1.0f;

		line->indices[i * 2 + 1 + offset] = i * 2 + 1 + offset;
	}

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusCreateCirclef(GLUSline* line, const GLUSfloat radius, const GLUSuint numberSectors)
{
	GLUSuint i;
	float singleStep;
	float totalStep;

	GLUSuint numberVertices = numberSectors;
	GLUSuint numberIndices = numberSectors;

	if (!line || numberSectors < 3 || numberVertices > GLUS_MAX_VERTICES || numberIndices > GLUS_MAX_INDICES)
	{
		return GLUS_FALSE;
	}
	glusInitLinef(line);

	line->numberVertices = numberSectors;
	line->numberIndices = numberSectors;

	line->vertices = (GLUSfloat*)malloc(4 * line->numberVertices * sizeof(GLUSfloat));
	line->indices = (GLUSuint*)malloc(line->numberIndices * sizeof(GLUSuint));

	line->mode = GL_LINE_LOOP;

	if (!glusCheckLinef(line))
	{
		glusDestroyLinef(line);

		return GLUS_FALSE;
	}

	singleStep = 2.0f * GLUS_PI / (float)numberSectors;
	totalStep = 0.0f;
	for (i = 0; i < numberSectors; i++)
	{
		line->vertices[0 + i * 4] = radius * cosf(totalStep);
		line->vertices[1 + i * 4] = radius * sinf(totalStep);
		line->vertices[2 + i * 4] = 0.0f;
		line->vertices[3 + i * 4] = 1.0f;

		line->indices[i] = i;

		totalStep += singleStep;
	}

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusCopyLinef(GLUSline* line, const GLUSline* source)
{
	if (!line || !source)
	{
		return GLUS_FALSE;
	}

	glusInitLinef(line);

	line->numberVertices = source->numberVertices;
	line->numberIndices = source->numberIndices;

	if (source->numberVertices)
	{
		line->vertices = (GLUSfloat*)malloc(4 * source->numberVertices * sizeof(GLUSfloat));
		if (!line->vertices)
		{
			glusDestroyLinef(line);

			return GLUS_FALSE;
		}
		memcpy(line->vertices, source->vertices, 4 * source->numberVertices * sizeof(GLUSfloat));
	}

	if (source->indices)
	{
		line->indices = (GLUSuint*)malloc(source->numberIndices * sizeof(GLUSuint));
		if (!line->indices)
		{
			glusDestroyLinef(line);

			return GLUS_FALSE;
		}
		memcpy(line->indices, source->indices, source->numberIndices * sizeof(GLUSuint));
	}

	line->mode = source->mode;

	return GLUS_TRUE;
}

GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyLinef(GLUSline* line)
{
	if (!line)
	{
		return;
	}

	if (line->vertices)
	{
		free(line->vertices);

		line->vertices = 0;
	}

	if (line->indices)
	{
		free(line->indices);

		line->indices = 0;
	}

	line->numberVertices = 0;
	line->numberIndices = 0;
	line->mode = 0;
}

