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

#ifndef __glus_h_
#define __glus_h_

#ifdef __cplusplus
extern "C"
{
#endif

#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <VG/openvg.h>
#include <EGL/egl.h>

#ifdef _MSC_VER
	#define GLUSINLINE static __forceinline
#else
	#define GLUSINLINE static inline
#endif

#ifndef GLUSAPIENTRY
#define GLUSAPIENTRY
#endif
#ifndef GLUSAPIENTRYP
#define GLUSAPIENTRYP GLUSAPIENTRY *
#endif
#ifndef GLUSAPI
#define GLUSAPI extern
#endif

typedef unsigned int GLUSenum;
typedef unsigned char GLUSboolean;
typedef unsigned int GLUSbitfield;
typedef signed char GLUSbyte;
typedef short GLUSshort;
typedef int GLUSint;
typedef int GLUSsizei;
typedef unsigned char GLUSubyte;
typedef unsigned short GLUSushort;
typedef unsigned int GLUSuint;
typedef float GLUSfloat;
typedef float GLUSclampf;
typedef double GLUSdouble;
typedef double GLUSclampd;
typedef char GLUSchar;

typedef struct _GLUScomplex
{
	float real;
	float imaginary;
} GLUScomplex;

#ifdef __cplusplus
#define GLUSvoid void
#else
typedef void GLUSvoid;
#endif

#define GLUS_OK 	0
#define GLUS_TRUE   1
#define GLUS_FALSE  0

#define GLUS_ALPHA  					0x00001906
#define GLUS_RGB    					0x00001907
#define GLUS_RGBA   					0x00001908
#define GLUS_LUMINANCE  				0x00001909

#define GLUS_COMPRESSED_R11_EAC                            0x9270
#define GLUS_COMPRESSED_SIGNED_R11_EAC                     0x9271
#define GLUS_COMPRESSED_RG11_EAC                           0x9272
#define GLUS_COMPRESSED_SIGNED_RG11_EAC                    0x9273
#define GLUS_COMPRESSED_RGB8_ETC2                          0x9274
#define GLUS_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2      0x9276
#define GLUS_COMPRESSED_RGBA8_ETC2_EAC                     0x9278

#define GLUS_PI		3.1415926535897932384626433832795f

#define GLUS_LOG_NOTHING	0
#define GLUS_LOG_ERROR   	1
#define GLUS_LOG_WARNING 	2
#define GLUS_LOG_INFO    	3
#define GLUS_LOG_DEBUG   	4
#define GLUS_LOG_SEVERE		5

#define GLUS_VERTICES_FACTOR 4
#define GLUS_VERTICES_DIVISOR 4

#define GLUS_MAX_VERTICES 65536
#define GLUS_MAX_INDICES  (GLUS_MAX_VERTICES*GLUS_VERTICES_FACTOR)

#define GLUS_MAX_STRING  256

#define GLUS_DEFAULT_CLIENT_VERSION 1

#define GLUS_EGL_API EGL_OPENVG_API

/**
 * Structure used for text file loading.
 */
typedef struct _GLUStextfile
{
	/**
	 * Contains the data of the text file.
	 */
    GLUSchar* text;

    /**
     * The length of the text file without the null terminator.
     */
    GLUSint length;

} GLUStextfile;

/**
 * Structure used for binary file loading.
 */
typedef struct _GLUSbinaryfile
{
    /**
     * The binary data of the file.
     */
    GLUSubyte* binary;

    /**
     * The length of the binary data.
     */
    GLUSint length;

} GLUSbinaryfile;

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

//
// EGL helper functions.
//

#include "../GLUS/glus_egl.h"

//
// Callback functions.
//

#include "../GLUS/glus_callback.h"

//
// Window preparation and creation functions.
//

#include "../GLUS/glus_os_wrapper_es.h"

//
// Extension functions.
//

#include "../GLUS/glus_extension.h"

//
// File, image loading functions.
//

#include "../GLUS/glus_load.h"

//
// File, image saving functions
//

#include "../GLUS/glus_save.h"

//
// Screenshot functions
//

#include "../GLUS/glus_screenshot.h"

//
// Perlin noise functions.
//

#include "../GLUS/glus_perlin.h"

//
// Vector functions.
//

#include "../GLUS/glus_vector.h"

//
// Point functions.
//

#include "../GLUS/glus_point.h"

//
// Plane functions.
//

#include "../GLUS/glus_plane.h"

//
// Sphere functions.
//

#include "../GLUS/glus_sphere.h"

//
// Axis Aligned Box functions.
//

#include "../GLUS/glus_axis_aligned_box.h"

//
// Oriented Box functions.
//

#include "../GLUS/glus_oriented_box.h"

//
// Math functions
//

#include "../GLUS/glus_math.h"

//
// Random functions
//

#include "../GLUS/glus_random.h"

//
// View, projection etc. functions.
//

#include "../GLUS/glus_viewprojection.h"

//
// Functions, related to planar surfaces.
//

#include "../GLUS/glus_planar.h"

//
// Matrix functions.
//

#include "../GLUS/glus_matrix.h"

//
// Quaternion functions.
//

#include "../GLUS/glus_quaternion.h"

//
// Complex numbers and vector functions.
//

#include "../GLUS/glus_complex.h"

#include "../GLUS/glus_complex_matrix.h"

#include "../GLUS/glus_complex_vector.h"

//
// Fourier functions.
//

#include "../GLUS/glus_fourier.h"

//
// Logging
//

#include "../GLUS/glus_log.h"

//
// Profiling
//

#include "../GLUS/glus_profile.h"

//
// Ray tracing
//

#include "../GLUS/glus_raytrace.h"

//
// Intersection testing
//

#include "../GLUS/glus_intersect.h"

//
// Texture sampling
//

#include "../GLUS/glus_texture.h"

//
// Padding
//

#include "../GLUS/glus_padding.h"

//
// Internally, some GLFW functions are used. See copyright informations in C file.
//

#include "../GLUS/glus_glfw_es.h"

#ifdef __cplusplus
}
#endif

#endif /*__glus_h_*/
