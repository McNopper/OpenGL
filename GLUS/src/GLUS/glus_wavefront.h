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

#ifndef GLUS_WAVEFRONT_H_
#define GLUS_WAVEFRONT_H_

/**
 * Loads a wavefront object file.
 *
 * @param filename The name of the wavefront file including extension.
 * @param shape The data is stored into this structure.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLoadObjFile(const GLUSchar* filename, GLUSshape* shape);

/**
 * Destroys the wavefront structure by freeing the allocated memory. VBOs, VAOs and textures are not freed.
 *
 * @param wavefront The structure which contains the dynamic allocated wavefront data, which will be freed by this function.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyGroupedObj(GLUSwavefront* wavefront);

/**
 * Loads a wavefront object file with groups and materials.
 *
 * @param filename The name of the wavefront file including extension.
 * @param wavefront The data is stored into this structure.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLoadGroupedObjFile(const GLUSchar* filename, GLUSwavefront* wavefront);

#endif /* GLUS_WAVEFRONT_H_ */
