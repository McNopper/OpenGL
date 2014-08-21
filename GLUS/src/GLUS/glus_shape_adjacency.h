/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) 2010 - 2014 Norbert Nopper
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

#ifndef GLUS_SHAPE_ADJACENCY_H_
#define GLUS_SHAPE_ADJACENCY_H_

/**
 * Creates a shape with adjacent index data for a triangle. Can be used in the geometry shader.
 *
 * @param adjacencyShape 	The shape with additional adjacent index data.
 * @param sourceShape 		The source shape.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusShapeCreateAdjacencyIndicesf(GLUSshape* adjacencyShape, const GLUSshape* sourceShape);

#endif /* GLUS_SHAPE_ADJACENCY_H_ */
