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

#ifndef GLUS_LINE_H_
#define GLUS_LINE_H_

/**
 * Creates a line out of two points.
 *
 * @param line The data is stored into this structure.
 * @param point0 The starting point.
 * @param point1 The ending point.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateLinef(GLUSline* line, const GLUSfloat point0[4], const GLUSfloat point1[4]);

/**
 * Creates a square out of lines.
 *
 * @param line The data is stored into this structure.
 * @param halfExtend The length from the center point to edge of the square.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateSquaref(GLUSline* line, const GLUSfloat halfExtend);

/**
 * Creates a rectangular grid out of lines. The grid is centered.
 *
 * @param line The data is stored into this structure.
 * @param horizontalExtend The width of the grid.
 * @param verticalExtend The height of the grid.
 * @param rows Number of rows of the grid.
 * @param columns Number of columns of the grid.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateRectangularGridf(GLUSline* line, const GLUSfloat horizontalExtend, const GLUSfloat verticalExtend, const GLUSuint rows, const GLUSuint columns);

/**
 * Creates a circle out of lines with the given radius and number sectors. More sectors makes the circle more round.
 *
 * @param line The data is stored into this structure.
 * @param radius The radius of the circle.
 * @param numberSectors The number of sectors the circle should have.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateCirclef(GLUSline* line, const GLUSfloat radius, const GLUSuint numberSectors);

/**
 * Copies the line.
 *
 * @param line The target line.
 * @param source The source line.
 *
 * @return GLUS_TRUE, if copy succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCopyLinef(GLUSline* line, const GLUSline* source);

/**
 * Destroys the line by freeing the allocated memory.
 *
 * @param line The structure which contains the dynamic allocated line data, which will be freed by this function.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyLinef(GLUSline* line);

#endif /* GLUS_LINE_H_ */
