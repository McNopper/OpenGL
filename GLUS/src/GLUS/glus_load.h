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

#ifndef GLUS_LOAD_H_
#define GLUS_LOAD_H_

/**
 * Loads a text file.
 *
 * @param filename The name of the file to load.
 * @param textfile The structure to fill the text data.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLoadTextFile(const GLUSchar* filename, GLUStextfile* textfile);

/**
 * Destroys the content of a text structure. Has to be called for freeing the resources.
 *
 * @param textfile The text file structure.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyTextFile(GLUStextfile* textfile);

/**
 * Loads a binary file.
 *
 * @param filename The name of the file to load.
 * @param binaryfile The structure to fill the binary data.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLoadBinaryFile(const GLUSchar* filename, GLUSbinaryfile* binaryfile);

/**
 * Destroys the content of a binary structure. Has to be called for freeing the resources.
 *
 * @param binaryfile The binary file structure.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyBinaryFile(GLUSbinaryfile* binaryfile);

/**
 * Loads a TGA file.
 *
 * @param filename The name of the file to load.
 * @param tgaimage The structure to fill the TGA data.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLoadTgaImage(const GLUSchar* filename, GLUStgaimage* tgaimage);

/**
 * Creates a TGA image.
 *
 * @param tgaimage	The structure to fill the TGA data.
 * @param width 	Width of the image.
 * @param height 	Height of the image.
 * @param depth 	Depth of the image.
 * @param format 	Format of the image.
 *
 * @return GLUS_TRUE, if creating succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateTgaImage(GLUStgaimage* tgaimage, GLUSint width, GLUSint height, GLUSint depth, GLUSenum format);

/**
 * Destroys the content of a TGA structure. Has to be called for freeing the resources.
 *
 * @param tgaimage The TGA file structure.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyTgaImage(GLUStgaimage* tgaimage);

/**
 * Loads a HDR file.
 *
 * @param filename The name of the file to load.
 * @param hdrimage The structure to fill the HDR data.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLoadHdrImage(const GLUSchar* filename, GLUShdrimage* hdrimage);

/**
 * Creates a HDR image.
 *
 * @param hdrimage	The structure to fill the HDR data.
 * @param width 	Width of the image.
 * @param height 	Height of the image.
 * @param depth 	Depth of the image.
 * @param format 	Format of the image.
 *
 * @return GLUS_TRUE, if creating succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateHdrImage(GLUShdrimage* hdrimage, GLUSint width, GLUSint height, GLUSint depth, GLUSenum format);

/**
 * Destroys the content of a HDR structure. Has to be called for freeing the resources.
 *
 * @param hdrimage The HDR file structure.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyHdrImage(GLUShdrimage* hdrimage);

/**
 * Loads a PKM image.
 *
 * @param filename			The name of the file to load.
 * @param pkmimage			The structure to fill the PKM data.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLoadPkmImage(const GLUSchar* filename, GLUSpkmimage* pkmimage);

/**
 * Destroys the content of a PKM structure. Has to be called for freeing the resources.
 *
 * @param pkmimage The PKM file structure.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyPkmImage(GLUSpkmimage* pkmimage);

#endif /* GLUS_LOAD_H_ */
