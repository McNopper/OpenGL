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

#define GLUS_POINT_TOLERANCE 0.001f

static GLUSboolean glusCheckShapef(GLUSshape* shape)
{
	if (!shape)
	{
		return GLUS_FALSE;
	}

	return shape->vertices && shape->normals && shape->tangents && shape->bitangents && shape->texCoords && shape->allAttributes && shape->indices;
}

static GLUSboolean glusFindIndexByIndicesf(GLUSuint* adjacentIndex, GLUSuint triangleFirstIndex, GLUSuint edge, const GLUSshape* shape)
{
	GLUSuint i, k, m;

	GLUSuint equalIndices;

	GLUSuint edgeIndices[2];

	if (!adjacentIndex || !shape)
	{
		return GLUS_FALSE;
	}

	if (shape->mode != GL_TRIANGLES)
	{
		return GLUS_FALSE;
	}

	for (i = 0; i < shape->numberIndices / 3; i++)
	{
		// Skip same triangle
		if (triangleFirstIndex == i * 3)
		{
			continue;
		}

		equalIndices = 0;

		for (k = 0; k < 3; k++)
		{
			for (m = 0 + edge; m < 2 + edge; m++)
			{
				if (shape->indices[triangleFirstIndex + (m % 3)] == shape->indices[i * 3 + k])
				{
					equalIndices++;

					edgeIndices[m - edge] = shape->indices[i * 3 + k];

					break;
				}
			}
		}

		if (equalIndices == 2)
		{
			for (k = 0; k < 3; k++)
			{
				if (shape->indices[i * 3 + k] != edgeIndices[0] && shape->indices[i * 3 + k] != edgeIndices[1])
				{
					*adjacentIndex = shape->indices[i * 3 + k];

					break;
				}
			}

			return GLUS_TRUE;
		}
	}

	return GLUS_FALSE;
}

static GLUSboolean glusFindIndexByVerticesf(GLUSuint* adjacentIndex, GLUSuint triangleIndex, GLUSuint edge, const GLUSshape* shape)
{
	GLUSuint i, k, m;

	GLUSuint equalVertices, walkerIndex, searchIndex;

	GLUSuint edgeIndices[2];

	if (!adjacentIndex || !shape)
	{
		return GLUS_FALSE;
	}

	if (shape->mode != GL_TRIANGLES)
	{
		return GLUS_FALSE;
	}

	for (i = 0; i < shape->numberIndices / 3; i++)
	{
		// Skip same triangle
		if (triangleIndex == i * 3)
		{
			continue;
		}

		equalVertices = 0;

		for (k = 0; k < 3; k++)
		{
			walkerIndex = shape->indices[i * 3 + k];

			for (m = 0 + edge; m < 2 + edge; m++)
			{
				searchIndex = shape->indices[triangleIndex + (m % 3)];

				if (shape->vertices[4 * searchIndex + 0] >= shape->vertices[4 * walkerIndex + 0] - GLUS_POINT_TOLERANCE && shape->vertices[4 * searchIndex + 0] <= shape->vertices[4 * walkerIndex + 0] + GLUS_POINT_TOLERANCE &&
						shape->vertices[4 * searchIndex + 1] >= shape->vertices[4 * walkerIndex + 1] - GLUS_POINT_TOLERANCE && shape->vertices[4 * searchIndex + 1] <= shape->vertices[4 * walkerIndex + 1] + GLUS_POINT_TOLERANCE &&
						shape->vertices[4 * searchIndex + 2] >= shape->vertices[4 * walkerIndex + 2] - GLUS_POINT_TOLERANCE && shape->vertices[4 * searchIndex + 2] <= shape->vertices[4 * walkerIndex + 2] + GLUS_POINT_TOLERANCE)
				{
					equalVertices++;

					edgeIndices[m - edge] = walkerIndex;

					break;
				}
			}
		}

		if (equalVertices == 2)
		{
			for (k = 0; k < 3; k++)
			{
				if (shape->indices[i * 3 + k] != edgeIndices[0] && shape->indices[i * 3 + k] != edgeIndices[1])
				{
					*adjacentIndex = shape->indices[i * 3 + k];

					break;
				}
			}

			return GLUS_TRUE;
		}
	}

	return GLUS_FALSE;
}

GLUSboolean GLUSAPIENTRY glusCreateAdjacencyShapef(GLUSshape* adjacencyShape, const GLUSshape* sourceShape)
{
	GLUSuint i;

	GLUSuint numberIndices;

	GLUSuint adjacentIndex, edge;

	if (!adjacencyShape || !sourceShape)
	{
		return GLUS_FALSE;
	}

	if (sourceShape->mode != GL_TRIANGLES)
	{
		return GLUS_FALSE;
	}

	numberIndices = sourceShape->numberIndices * 2;

	if (numberIndices > GLUS_MAX_INDICES)
	{
		return GLUS_FALSE;
	}

	if (!glusCopyShapef(adjacencyShape, sourceShape))
	{
		glusDestroyShapef(adjacencyShape);

		return GLUS_FALSE;
	}

	adjacencyShape->numberIndices = numberIndices;

	glusFree(adjacencyShape->indices);
	adjacencyShape->indices = (GLUSuint*)glusMalloc(numberIndices * sizeof(GLUSuint));

	adjacencyShape->mode = GL_TRIANGLES_ADJACENCY;

	if (!glusCheckShapef(adjacencyShape))
	{
		glusDestroyShapef(adjacencyShape);

		return GLUS_FALSE;
	}

	// Process all triangles
	for (i = 0; i < sourceShape->numberIndices / 3; i++)
	{
		// For now, all adjacent triangles are degenerated.
		adjacencyShape->indices[6 * i + 0] = sourceShape->indices[3 * i + 0];
		adjacencyShape->indices[6 * i + 1] = sourceShape->indices[3 * i + 0];
		adjacencyShape->indices[6 * i + 2] = sourceShape->indices[3 * i + 1];
		adjacencyShape->indices[6 * i + 3] = sourceShape->indices[3 * i + 1];
		adjacencyShape->indices[6 * i + 4] = sourceShape->indices[3 * i + 2];
		adjacencyShape->indices[6 * i + 5] = sourceShape->indices[3 * i + 2];

		// Skip degenerated triangles
		if (sourceShape->indices[i * 3 + 0] == sourceShape->indices[i * 3 + 1] || sourceShape->indices[i * 3 + 0] == sourceShape->indices[i * 3 + 2] || sourceShape->indices[i * 3 + 1] == sourceShape->indices[i * 3 + 2])
		{
			continue;
		}

		// Find adjacent indices for each edge of the triangle ...
		for (edge = 0; edge < 3; edge++)
		{
			adjacentIndex = 0;

			// ... by scanning the indices ...
			if (glusFindIndexByIndicesf(&adjacentIndex, 3 * i, edge, sourceShape))
			{
				adjacencyShape->indices[6 * i + edge * 2 + 1] = adjacentIndex;

				continue;
			}

			// ... and if not found, compare the vertices.
			if (glusFindIndexByVerticesf(&adjacentIndex, 3 * i, edge, sourceShape))
			{
				adjacencyShape->indices[6 * i + edge * 2 + 1] = adjacentIndex;

				continue;
			}

			glusLogPrint(GLUS_LOG_WARNING, "Triangle %d with edge %d: No adjacent index found!", i, edge);
		}
	}

	return GLUS_TRUE;
}
