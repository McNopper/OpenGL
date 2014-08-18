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

#ifndef GLUS_LINE_USHORT_H_
#define GLUS_LINE_USHORT_H_

/**
 * Structure for holding line data.
 */
typedef struct _GLUSline
{
	/**
	 * Vertices in homogeneous coordinates.
	 */
    GLUSfloat* vertices;

    /**
     * Indices.
     */
    GLUSushort* indices;

    /**
     * Number of vertices.
     */
    GLUSushort numberVertices;

    /**
     * Number of indices.
     */
    GLUSuint numberIndices;

    /**
     * Line render mode - could be either:
     *
     * GL_LINES
     * GL_LINE_STRIP
     * GL_LINE_LOOP
     */
    GLUSenum mode;

} GLUSline;

#endif /* GLUS_LINE_USHORT_H_ */
