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

#ifndef GLUS_IMAGE_PKM_H_
#define GLUS_IMAGE_PKM_H_

/**
 * Loads a PKM image.
 *
 * @param filename			The name of the file to load.
 * @param pkmimage			The structure to fill the PKM data.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusImageLoadPkm(const GLUSchar* filename, GLUSpkmimage* pkmimage);

/**
 * Destroys the content of a PKM structure. Has to be called for freeing the resources.
 *
 * @param pkmimage The PKM file structure.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusImageDestroyPkm(GLUSpkmimage* pkmimage);

#endif /* GLUS_IMAGE_PKM_H_ */
