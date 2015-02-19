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

#ifndef GLUS_LINE_WAVEFRONT_H_
#define GLUS_LINE_WAVEFRONT_H_

/**
 * Loads a wavefront object file containing only line information.
 *
 * @param filename The name of the wavefront file including extension.
 * @param line The data is stored into this structure.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLineLoadWavefront(const GLUSchar* filename, GLUSline* line);

#endif /* GLUS_SHAPE_WAVEFRONT_H_ */
