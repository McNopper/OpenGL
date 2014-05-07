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

#ifndef GLUS_TEXTURE_H_
#define GLUS_TEXTURE_H_

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
GLUSAPI GLUSboolean GLUSAPIENTRY glusTexImage2DSampleHdrImage(GLUSfloat rgb[3], const GLUShdrimage* hdrimage, const GLUSfloat st[2]);

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
GLUSAPI GLUSboolean GLUSAPIENTRY glusTexImage2DSampleTgaImage(GLUSubyte rgba[4], const GLUStgaimage* tgaimage, const GLUSfloat st[2]);

#endif /* GLUS_TEXTURE_H_ */
