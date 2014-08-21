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

#ifndef GLUS_IMAGE_H_
#define GLUS_IMAGE_H_

/**
 * Structure used for Targa Image File loading.
 */
typedef struct _GLUStgaimage
{
	/**
	 * Width of the TGA image.
	 */
    GLUSushort width;

	/**
	 * Height of the TGA image.
	 */
    GLUSushort height;

	/**
	 * Depth of the image.
	 */
    GLUSushort depth;

    /**
     * Pixel data.
     */
    GLUSubyte* data;

    /**
     * Format of the TGA image. Can be:
     *
	 * GLUS_RGB
     * GLUS_RGBA
     * GLUS_LUMINANCE
     *
     * Last entry can also be interpreted as GLUS_ALPHA.
     */
    GLUSenum format;

} GLUStgaimage;

/**
 * Structure used for HDR Image File loading.
 */
typedef struct _GLUShdrimage
{
	/**
	 * Width of the HDR image.
	 */
	GLUSushort width;

	/**
	 * Height of the HDR image.
	 */
	GLUSushort height;

	/**
	 * Depth of the image.
	 */
	GLUSushort depth;

	/**
	 * Pixel data.
	 */
	GLUSfloat* data;

    /**
     * Format of the HDR image.
     */
    GLUSenum format;

} GLUShdrimage;

/**
 * Structure used for PKM Image File loading.
 */
typedef struct _GLUSpkmimage
{
	/**
	 * Width of the PKM image.
	 */
	GLUSushort width;

	/**
	 * Height of the PKM image.
	 */
	GLUSushort height;

	/**
	 * Depth of the image.
	 */
	GLUSushort depth;

	/**
	 * Pixel data.
	 */
	GLUSubyte* data;

    /**
     * The image size in bytes.
     */
    GLUSint imageSize;

    /**
     * Internal format of the PKM image.
     */
    GLUSenum internalformat;

} GLUSpkmimage;

#endif /* GLUS_IMAGE_H_ */
