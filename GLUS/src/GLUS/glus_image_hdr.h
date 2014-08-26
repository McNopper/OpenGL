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

#ifndef GLUS_IMAGE_HDR_H_
#define GLUS_IMAGE_HDR_H_

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
GLUSAPI GLUSboolean GLUSAPIENTRY glusImageCreateHdr(GLUShdrimage* hdrimage, GLUSint width, GLUSint height, GLUSint depth, GLUSenum format);

/**
 * Loads a HDR file.
 *
 * @param filename The name of the file to load.
 * @param hdrimage The structure to fill the HDR data.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusImageLoadHdr(const GLUSchar* filename, GLUShdrimage* hdrimage);

/**
 * Saves a HDR file.
 *
 * @param filename The name of the file to save.
 * @param hdrimage The structure with the HDR data.
 *
 * @return GLUS_TRUE, if saving succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusImageSaveHdr(const GLUSchar* filename, const GLUShdrimage* hdrimage);

/**
 * Destroys the content of a HDR structure. Has to be called for freeing the resources.
 *
 * @param hdrimage The HDR file structure.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusImageDestroyHdr(GLUShdrimage* hdrimage);

/**
 * Samples a RGB color value from a HDR 2D image.
 * Sampling uses a bilinear filter.
 *
 * @param rgb 		The resulting, sampled RGB color value.
 * @param hdrimage 	The HDR image structure, containing the 2D texel data.
 * @param st		Texture coordinate, where to sample the 2D texture.
 *
 * @return GLUS_TRUE, if sampling succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusImageSampleHdr2D(GLUSfloat rgb[3], const GLUShdrimage* hdrimage, const GLUSfloat st[2]);

#endif /* GLUS_IMAGE_HDR_H_ */
