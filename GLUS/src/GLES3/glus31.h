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

#include <GLES3/gl31.h>
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

#define GLUS_VERTEX_SHADER              0x00008B31
#define GLUS_FRAGMENT_SHADER            0x00008B30
#define GLUS_COMPUTE_SHADER             0x000091B9

#define GLUS_VERTEX_SHADER_BIT 			0x00000001
#define GLUS_FRAGMENT_SHADER_BIT 		0x00000002
#define GLUS_COMPUTE_SHADER_BIT 		0x00000020
#define GL_ALL_SHADER_BITS 				0xFFFFFFFF

#define GLUS_RED  						0x00001903
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

#define GLUS_MAX_VERTICES 1048576
#define GLUS_MAX_INDICES  (GLUS_MAX_VERTICES*GLUS_VERTICES_FACTOR)

#define GLUS_MAX_STRING  256

#define GLUS_DEFAULT_CLIENT_VERSION 3

#define GLUS_EGL_API EGL_OPENGL_ES_API

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

/**
 * Structure for shader program handling.
 */
typedef struct _GLUSshaderprogram
{
	/**
	 * The created program.
	 */
    GLUSuint program;

    /**
     * Compute shader.
     */
    GLUSuint compute;

    /**
     * Vertex shader.
     */
    GLUSuint vertex;

    /**
     * Fragment shader.
     */
    GLUSuint fragment;

} GLUSshaderprogram;

/**
 * Structure for program pipeline handling.
 */
typedef struct _GLUSprogrampipeline
{
	/**
	 * The created pipeline.
	 */
    GLUSuint pipeline;

    /**
     * Compute shader program.
     */
    GLUSuint computeProgram;

    /**
     * Vertex shader program.
     */
    GLUSuint vertexProgram;

    /**
     * Fragment shader program.
     */
    GLUSuint fragmentProgram;

} GLUSprogrampipeline;

/**
 * Structure for holding geometry data.
 */
typedef struct _GLUSshape
{
	/**
	 * Vertices in homogeneous coordinates.
	 */
    GLUSfloat* vertices;

    /**
     * Normals.
     */
    GLUSfloat* normals;

    /**
     * Tangents.
     */
    GLUSfloat* tangents;

    /**
     * Bitangents.
     */
    GLUSfloat* bitangents;

    /**
     * Texture coordinates.
     */
    GLUSfloat* texCoords;

    /**
     * All above values in one array. Not created by the model loader.
     */
    GLUSfloat* allAttributes;

    /**
     * Indices.
     */
    GLUSuint* indices;

    /**
     * Number of vertices.
     */
    GLUSuint numberVertices;

    /**
     * Number of indices.
     */
    GLUSuint numberIndices;

    /**
     * Triangle render mode - could be either:
     *
     * GL_TRIANGLES
     * GL_TRIANGLE_STRIP
     */
    GLUSenum mode;

} GLUSshape;

/**
 * Structure for holding material data.
 */
typedef struct _GLUSmaterial
{
	/**
	 * Name of the material.
	 */
	GLUSchar name[GLUS_MAX_STRING];

	/**
	 * Emissive color.
	 */
	GLUSfloat emissive[4];

	/**
	 * Ambient color.
	 */
	GLUSfloat ambient[4];

	/**
	 * Diffuse color.
	 */
	GLUSfloat diffuse[4];

	/**
	 * Specular color.
	 */
	GLUSfloat specular[4];

	/**
	 * Shininess.
	 */
	GLUSfloat shininess;

	/**
	 * Transparency, which is the alpha value.
	 */
	GLUSfloat transparency;

	/**
	 * Reflection.
	 */
	GLUSboolean reflection;

	/**
	 * Refraction.
	 */
	GLUSboolean refraction;

	/**
	 * Index of refraction.
	 */
	GLUSfloat indexOfRefraction;

	/**
	 * Ambient color texture filename.
	 */
	GLUSchar ambientTextureFilename[GLUS_MAX_STRING];

	/**
	 * Diffuse color texture filename.
	 */
	GLUSchar diffuseTextureFilename[GLUS_MAX_STRING];

	/**
	 * Specular color texture filename.
	 */
	GLUSchar specularTextureFilename[GLUS_MAX_STRING];

	/**
	 * Transparency texture filename.
	 */
	GLUSchar transparencyTextureFilename[GLUS_MAX_STRING];

	/**
	 * Bump texture filename.
	 */
	GLUSchar bumpTextureFilename[GLUS_MAX_STRING];

	/**
	 * Can be used to store the ambient texture name.
	 */
    GLUSuint ambientTextureName;

	/**
	 * Can be used to store the diffuse texture name.
	 */
    GLUSuint diffuseTextureName;

	/**
	 * Can be used to store the specular texture name.
	 */
    GLUSuint specularTextureName;

	/**
	 * Can be used to store the transparency texture name.
	 */
    GLUSuint transparencyTextureName;

	/**
	 * Can be used to store the bump texture name.
	 */
    GLUSuint bumpTextureName;

} GLUSmaterial;

/**
 * Structure for holding material data list.
 */
typedef struct _GLUSmaterialList
{
	/**
	 * The material data.
	 */
	GLUSmaterial material;

	/**
	 * The pointer to the next element.
	 */
	struct _GLUSmaterialList* next;

} GLUSmaterialList;

/**
 * Group of geometry.
 */
typedef struct _GLUSgroup
{
		/**
		 * Name of the group.
		 */
		GLUSchar name[GLUS_MAX_STRING];

		/**
		 * Name of the material.
		 */
		GLUSchar materialName[GLUS_MAX_STRING];

		/**
		 * Pointer to the material.
		 */
		GLUSmaterial* material;

	    /**
	     * Indices.
	     */
	    GLUSuint* indices;

	    /**
	     * Indices VBO.
	     */
	    GLUSuint indicesVBO;

	    /**
	     * VAO of this group.
	     */
	    GLUSuint vao;

	    /**
	     * Number of indices.
	     */
	    GLUSuint numberIndices;

	    /**
	     * Triangle render mode - could be either:
	     *
	     * GL_TRIANGLES
	     * GL_TRIANGLE_STRIP
	     */
	    GLUSenum mode;

} GLUSgroup;

/**
 * Structure for holding the group data list.
 */
typedef struct _GLUSgroupList
{
		/**
		 * The group data.
		 */
		GLUSgroup group;

	    /**
	     * The pointer to the next group element.
	     */
	    struct _GLUSgroupList* next;

} GLUSgroupList;

/**
 * Structure for a complete wavefront object file.
 */
typedef struct _GLUSwavefront
{
		/**
		 * Vertices in homogeneous coordinates.
		 */
	    GLUSfloat* vertices;

	    /**
	     * Vertices VBO.
	     */
	    GLUSuint verticesVBO;

	    /**
	     * Normals.
	     */
	    GLUSfloat* normals;

	    /**
	     * Normals VBO.
	     */
	    GLUSuint normalsVBO;

	    /**
	     * Tangents.
	     */
	    GLUSfloat* tangents;

	    /**
	     * Tangents VBO.
	     */
	    GLUSuint tangentsVBO;

	    /**
	     * Bitangents.
	     */
	    GLUSfloat* bitangents;

	    /**
	     * Bitangents VBO.
	     */
	    GLUSuint bitangentsVBO;

	    /**
	     * Texture coordinates.
	     */
	    GLUSfloat* texCoords;

	    /**
	     * Texture corrdinates VBO.
	     */
	    GLUSuint texCoordsVBO;

	    /**
	     * Number of vertices.
	     */
	    GLUSuint numberVertices;

	    /**
	     * Pointer to the first element of the groups.
	     */
	    GLUSgroupList* groups;

	    /**
	     * Pointer to the first element of the materials.
	     */
	    GLUSmaterialList* materials;

} GLUSwavefront;

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
    GLUSuint* indices;

    /**
     * Number of vertices.
     */
    GLUSuint numberVertices;

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
// Shader creation function.
//

#include "../GLUS/glus_shaderprogram_es31.h"

#include "../GLUS/glus_programpipeline_es31.h"

//
// Shape / geometry functions.
//

#include "../GLUS/glus_shape.h"

//
// Shape texture coordinate generation
//

#include "../GLUS/glus_shape_texgen.h"

//
// Line / geometry functions.
//

#include "../GLUS/glus_line.h"

//
// Model loading functions.
//

#include "../GLUS/glus_wavefront.h"

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
