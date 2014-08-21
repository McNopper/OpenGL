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

#ifndef GLUS_IMAGE_TGA_H_
#define GLUS_IMAGE_TGA_H_

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
GLUSAPI GLUSboolean GLUSAPIENTRY glusImageCreateTga(GLUStgaimage* tgaimage, GLUSint width, GLUSint height, GLUSint depth, GLUSenum format);

/**
 * Loads a TGA file.
 *
 * @param filename The name of the file to load.
 * @param tgaimage The structure to fill the TGA data.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusImageLoadTga(const GLUSchar* filename, GLUStgaimage* tgaimage);

/**
 * Saves a TGA file.
 *
 * @param filename The name of the file to save.
 * @param tgaimage The structure with the TGA data.
 *
 * @return GLUS_TRUE, if saving succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusImageSaveTga(const GLUSchar* filename, const GLUStgaimage* tgaimage);

/**
 * Destroys the content of a TGA structure. Has to be called for freeing the resources.
 *
 * @param tgaimage The TGA file structure.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusImageDestroyTga(GLUStgaimage* tgaimage);

/**
 * Samples a RGBA color value from a TGA 2D image.
 * Sampling uses a bilinear filter.
 *
 * @param rgba 		The resulting, sampled RGBA color value.
 * @param tgaimage 	The TGA image structure, containing the 2D texel data.
 * @param st		Texture coordinate, where to sample the 2D texture.
 *
 * @return GLUS_TRUE, if sampling succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusImageSampleTga2D(GLUSubyte rgba[4], const GLUStgaimage* tgaimage, const GLUSfloat st[2]);

#endif /* GLUS_IMAGE_TGA_H_ */
