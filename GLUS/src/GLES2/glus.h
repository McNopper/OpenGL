/*
 * GLUS - OpenGL ES 2.0 Utilities. Copyright (C) 2010 - 2013 Norbert Nopper
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

#include <GLES2/gl2.h>
#include <EGL/egl.h>

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

#ifdef __cplusplus
#define GLUSvoid void
#else
typedef void GLUSvoid;
#endif

#define GLUS_OK 0
#define GLUS_TRUE   1
#define GLUS_FALSE  0
#define GLUS_VERTEX_SHADER              0x8B31
#define GLUS_FRAGMENT_SHADER            0x8B30

#define GLUS_RED  		0x1903
#define GLUS_ALPHA  	0x1906
#define GLUS_RGB    	0x1907
#define GLUS_RGBA   	0x1908
#define GLUS_LUMINANCE  0x1909

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

#define GLUS_DEFAULT_CLIENT_VERSION 2

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
 * Structure for shader program handling.
 */
typedef struct _GLUSshaderprogram
{
	/**
	 * The created program.
	 */
    GLUSuint program;

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
    GLUSushort* indices;

    /**
     * Number of vertices.
     */
    GLUSushort numberVertices;

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
    GLUSushort* indices;

    /**
     * Number of vertices.
     */
    GLUSushort numberVertices;

    /**
     * Number of indices.
     */
    GLUSuint numberIndices;

    /**
     * Line render mode - could be either:
     *
     * GL_LINES​
     * GL_LINE_STRIP​
     * GL_LINE_LOOP​
     */
    GLUSenum mode;

} GLUSline;

//
// EGL helper functions.
//

/**
 * Creates the context and all needed EGL elements.
 *
 * @param eglNativeDisplayType		Native display type.
 * @param eglDisplay  				EGL display.
 * @param eglConfig  				EGL configuration.
 * @param eglContext 				EGL context.
 * @param attribList 				EGL attribute list.
 * @param eglContextClientVersion	EGL context client version.
 *
 * @return EGL_TRUE, when creation of context succeeded.
 */
GLUSAPI EGLBoolean GLUSAPIENTRY glusEGLCreateContext(EGLNativeDisplayType eglNativeDisplayType, EGLDisplay* eglDisplay, EGLConfig* eglConfig, EGLContext *eglContext, const EGLint attribList[], const EGLint eglContextClientVersion);

/**
 * Gathers the native visual ID. Needed on some platforms to initialize EGL
 *
 * @param eglDisplay  			EGL display.
 * @param eglConfig  			EGL configuration.
 * @param eglNativeVisualID 	EGL native visual ID.
 *
 * @return EGL_TRUE, when gathering succeeded.
 */
GLUSAPI EGLBoolean GLUSAPIENTRY glusEGLGetNativeVisualID(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLint* eglNativeVisualID);

/**
 * Creates a window surface and sets it as current.
 *
 * @param eglNativeWindowType   	Native window type.
 * @param eglDisplay 				EGL display.
 * @param eglConfig  				EGL configuration.
 * @param eglContext 				EGL context.
 * @param eglSurface 				EGL surface.
 *
 * @return EGL_TRUE, when creation of window and setting of context succeeded.
 */
GLUSAPI EGLBoolean GLUSAPIENTRY glusEGLCreateWindowSurfaceMakeCurrent(EGLNativeWindowType eglNativeWindowType, EGLDisplay* eglDisplay, EGLConfig* eglConfig, EGLContext* eglContext, EGLSurface* eglSurface);

/**
 * Terminates EGL an frees all resources.
 *
 * @param eglDisplay EGL display.
 * @param eglContext EGL context.
 * @param eglSurface EGL surface.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusEGLTerminate(EGLDisplay* eglDisplay, EGLContext* eglContext, EGLSurface* eglSurface);

//
// Callback functions.
//

/**
 * Sets the users initialization function. Called before the reshape and update functions.
 *
 * The function must return GLUS_TRUE, if the initialization succeeded. If not, the program terminates.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusInitFunc(GLUSboolean(*glusNewInit)(GLUSvoid));

/**
 * Sets the users reshape function. Called after the initialization and before the update function.
 * If the window is resized, this function is also called before the update function.
 *
 * The function receives the current width and height of the resized window.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusReshapeFunc(GLUSvoid(*glusNewReshape)(const GLUSint width, const GLUSint height));

/**
 * Sets the users update function, which is called every frame.
 *
 * The time difference since last frame is passed in seconds and the resolution is at least in milliseconds.
 *
 * If the function does not return GLUS_TRUE, the application is terminated.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusUpdateFunc(GLUSboolean(*glusNewUpdate)(const GLUSfloat time));

/**
 * Sets the users terminate function, which is called in any case. It can be used to clean up resources.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusTerminateFunc(GLUSvoid(*glusNewTerminate)(GLUSvoid));

/**
 * Sets the users key handler.
 *
 * The function receives the values, if a key was pressed or released.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusKeyFunc(GLUSvoid(*glusNewKey)(const GLUSboolean pressed, const GLUSint key));

/**
 * Sets the users mouse handler.
 *
 * The function receives the state of a button and the x and y position in the window.
 *
 * Buttons are:
 * 1 Left mouse button.
 * 2 Middle mouse button.
 * 4 Right mouse button.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMouseFunc(GLUSvoid(*glusNewMouse)(const GLUSboolean pressed, const GLUSint button, const GLUSint xPos, const GLUSint yPos));

/**
 * Sets the users mouse wheel function.
 *
 * The function receives the states of all buttons, the direction of the wheel in ticks and the current x and y position.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMouseWheelFunc(GLUSvoid(*glusNewMouseWheel)(const GLUSint buttons, const GLUSint ticks, const GLUSint xPos, const GLUSint yPos));

/**
 * Sets the users mouse move function.
 *
 * The function receives the states of all buttons and the current x and y position.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMouseMoveFunc(GLUSvoid(*glusNewMouseMove)(const GLUSint buttons, const GLUSint xPos, const GLUSint yPos));

//
// Window preparation and creation functions.
//

/**
 * Prepare the EGL context. Must be called before windows creation. Default is 2.
 *
 * @param version EGL context version.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPrepareContext(const GLUSint version);

/**
 * Prepare the window. Must be called before windows creation.
 *
 * @param noResize GLUS_TRUE, if it should not be possible to resize the window.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPrepareNoResize(const GLUSboolean noResize);

/**
 * Creates the window. In this function, mainly GLEW and GLFW functions are used. By default, a RGBA color buffer is created.
 *
 * @param title Title of the window.
 * @param width Width of the window.
 * @param height Height of the window.
 * @param attribList EGL attribute list.
 * @param fullscreen Flag for setting the window to fullscreen.
 *
 * @return GLUS_TRUE, if creation of OPenGL context and window succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateWindow(const GLUSchar* title, const GLUSint width, const GLUSint height, const EGLint* attribList, const GLUSboolean fullscreen);

/**
 * Cleans up the window and frees all resources. Only needs to be called, if creation of the window failed.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyWindow(GLUSvoid);

/**
 * Starts the main loop. First the init, reshape, update and - if terminated - terminate functions are called.
 *
 * @return GLUS_ TRUE, if running the main loop succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusRun(GLUSvoid);

//
// Extension functions.
//

/**
 * Checks, if an OpenGL ES extension is supported.
 *
 * @param extension The name of the extension.
 *
 * @return GLUS_TRUE, if the given extension is supported.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusExtensionSupported(const GLUSchar* extension);

/**
 * Gathers the function pointer of an extension.
 *
 * @param procname The name of the function.
 *
 * @return The function pointer.
 */
GLUSAPI void* GLUSAPIENTRY glusGetProcAddress(const GLUSchar* procname);

//
// File, image loading functions.
//

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
 * Destroys the content of a TGA structure. Has to be called for freeing the resources.
 *
 * @param tgaimage The TGA file structure.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyTgaImage(GLUStgaimage* tgaimage);

//
// File, image saving functions
//

/**
 * Saves a TGA file.
 *
 * @param filename The name of the file to save.
 * @param tgaimage The structure with the TGA data.
 *
 * @return GLUS_TRUE, if saving succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusSaveTgaImage(const GLUSchar* filename, const GLUStgaimage* tgaimage);

//
// Screenshot functions
//

/**
 * Creates a screenshot of the current back buffer.
 *
 * @param x The lower left x coordinate from the screenshot rectangle.
 * @param y The lower left y coordinate from the screenshot rectangle.
 * @param width The width of the screenshot rectangle.
 * @param height The height of the screenshot rectangle.
 * @param screenshot The image structure to store the pixel data.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusScreenshot(GLUSint x, GLUSint y, GLUSsizei width, GLUSsizei height, GLUStgaimage* screenshot);

//
// Perlin noise functions.
//

/**
 * Creates a 1D perlin noise texture. See OpenGL Programming Guide 4.3, p.460ff
 *
 * @param image The perlin noise texture will be stored into this image.
 * @param width Width of the texture.
 * @param seed Random seed number.
 * @param frequency Frequency of the noise.
 * @param amplitude Amplitude of the noise.
 * @param persistence Persistence of the noise.
 * @param octaves Octaves of the noise.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreatePerlinNoise1D(GLUStgaimage* image, const GLUSint width, const GLUSint seed, const GLUSfloat frequency, const GLUSfloat amplitude, const GLUSfloat persistence, const GLUSint octaves);

/**
 * Creates a 2D perlin noise texture. See OpenGL Programming Guide 4.3, p.460ff
 *
 * @param image The perlin noise texture will be stored into this image.
 * @param width Width of the texture.
 * @param height Height of the texture
 * @param seed Random seed number.
 * @param frequency Frequency of the noise.
 * @param amplitude Amplitude of the noise.
 * @param persistence Persistence of the noise.
 * @param octaves Octaves of the noise.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreatePerlinNoise2D(GLUStgaimage* image, const GLUSint width, const GLUSint height, const GLUSint seed, const GLUSfloat frequency, const GLUSfloat amplitude, const GLUSfloat persistence, const GLUSint octaves);

/**
 * Creates a 3D perlin noise texture. See OpenGL Programming Guide 4.3, p.460ff
 *
 * @param image The perlin noise texture will be stored into this image.
 * @param width Width of the texture.
 * @param height Height of the texture
 * @param depth Depth of the texture
 * @param seed Random seed number.
 * @param frequency Frequency of the noise.
 * @param amplitude Amplitude of the noise.
 * @param persistence Persistence of the noise.
 * @param octaves Octaves of the noise.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreatePerlinNoise3D(GLUStgaimage* image, const GLUSint width, const GLUSint height, const GLUSint depth, const GLUSint seed, const GLUSfloat frequency, const GLUSfloat amplitude, const GLUSfloat persistence, const GLUSint octaves);

//
// Vector functions.
//

/**
 * Copies a 3D Vector.
 *
 * @param result The destination vector.
 * @param vector The source vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3Copyf(GLUSfloat result[3], const GLUSfloat vector[3]);

/**
 * Copies a 2D Vector.
 *
 * @param result The destination vector.
 * @param vector The source vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2Copyf(GLUSfloat result[2], const GLUSfloat vector[2]);

/**
 * Adds a 3D Vector to another.
 *
 * @param result The final vector.
 * @param vector0 The first vector.
 * @param vector1 The second vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3AddVector3f(GLUSfloat result[3], const GLUSfloat vector0[3], const GLUSfloat vector1[3]);

/**
 * Adds a 2D Vector to another.
 *
 * @param result The final vector.
 * @param vector0 The first vector.
 * @param vector1 The second vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2AddVector2f(GLUSfloat result[2], const GLUSfloat vector0[2], const GLUSfloat vector1[2]);

/**
 * Subtracts a 3D Vector from another.
 *
 * @param result The final vector.
 * @param vector0 The vector subtracted by vector1.
 * @param vector1 The vector subtracted from vector0.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3SubtractVector3f(GLUSfloat result[3], const GLUSfloat vector0[3], const GLUSfloat vector1[3]);

/**
 * Subtracts a 2D Vector from another.
 *
 * @param result The final vector.
 * @param vector0 The vector subtracted by vector1.
 * @param vector1 The vector subtracted from vector0.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2SubtractVector2f(GLUSfloat result[2], const GLUSfloat vector0[2], const GLUSfloat vector1[2]);

/**
 * Multiplies a 3D Vector by a scalar.
 *
 * @param result The final vector.
 * @param vector The used vector for multiplication.
 * @param scalar The scalar.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3MultiplyScalarf(GLUSfloat result[3], const GLUSfloat vector[3], const GLUSfloat scalar);

/**
 * Multiplies a 2D Vector by a scalar.
 *
 * @param result The final vector.
 * @param vector The used vector for multiplication.
 * @param scalar The scalar.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2MultiplyScalarf(GLUSfloat result[2], const GLUSfloat vector[2], const GLUSfloat scalar);

/**
 * Calculates the length of a 3D Vector.
 *
 * @param vector The used vector.
 *
 * @return The length of the vector.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusVector3Lengthf(const GLUSfloat vector[3]);

/**
 * Calculates the length of a 2D Vector.
 *
 * @param vector The used vector.
 *
 * @return The length of the vector.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusVector2Lengthf(const GLUSfloat vector[2]);

/**
 * Normalizes the given 3D Vector.
 *
 * @param vector The vector to normalize.
 *
 * @return GLUS_TRUE, if normalization succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusVector3Normalizef(GLUSfloat vector[3]);

/**
 * Normalizes the given 2D Vector.
 *
 * @param vector The vector to normalize.
 *
 * @return GLUS_TRUE, if normalization succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusVector2Normalizef(GLUSfloat vector[2]);

/**
 * Calculates the dot product of two 3D vectors.
 *
 * @param vector0 The first vector.
 * @param vector1 The second vector.
 *
 * @return The dot product.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusVector3Dotf(const GLUSfloat vector0[3], const GLUSfloat vector1[3]);

/**
 * Calculates the dot product of two 2D vectors.
 *
 * @param vector0 The first vector.
 * @param vector1 The second vector.
 *
 * @return The dot product.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusVector2Dotf(const GLUSfloat vector0[2], const GLUSfloat vector1[2]);

/**
 * Calculates the cross product of two 3D vectors: vector0 x vector1.
 *
 * @param result The resulting vector from the cross product.
 * @param vector0 The first vector.
 * @param vector1 The second vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3Crossf(GLUSfloat result[3], const GLUSfloat vector0[3], const GLUSfloat vector1[3]);

/**
 * Creates a quaternion out of a 3D vector.
 *
 * @param result The resulting quaternion.
 * @param vector The used vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3GetQuaternionf(GLUSfloat result[4], const GLUSfloat vector[3]);

/**
 * Creates a 3D point, given as homogeneous coordinates, out of a 3D vector.
 *
 * @param result The resulting point.
 * @param vector The used vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector3GetPoint4f(GLUSfloat result[4], const GLUSfloat vector[3]);

/**
 * Creates a 2D point, given as homogeneous coordinates, out of a 2D vector.
 *
 * @param result The resulting point.
 * @param vector The used vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVector2GetPoint3f(GLUSfloat result[3], const GLUSfloat vector[2]);

//
// Point functions.
//

/**
 * Copies a 3D point, given as homogeneous coordinates.
 *
 * @param result The destination point.
 * @param point The source point.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint4Copyf(GLUSfloat result[4], const GLUSfloat point[4]);

/**
 * Copies a 2D point, given as homogeneous coordinates.
 *
 * @param result The destination point.
 * @param point The source point.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint3Copyf(GLUSfloat result[3], const GLUSfloat point[3]);

/**
 * Subtracts a 3D point, given as homogeneous coordinates, from another and calculates a 3D vector.
 *
 * @param result The resulting vector.
 * @param point0 The point subtracted by point1.
 * @param point1 The point subtracted from point0.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint4SubtractPoint4f(GLUSfloat result[3], const GLUSfloat point0[4], const GLUSfloat point1[4]);

/**
 * Subtracts a 2D point, given as homogeneous coordinates, from another and calculates a 2D vector.
 *
 * @param result The resulting vector.
 * @param point0 The point subtracted by point1.
 * @param point1 The point subtracted from point0.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint3SubtractPoint3f(GLUSfloat result[2], const GLUSfloat point0[3], const GLUSfloat point1[3]);

/**
 * Adds a vector to a 3D point, given as homogeneous coordinates. Result is the new point.
 *
 * @param result The resulting point.
 * @param point The point.
 * @param vector The vector, which is added to the point.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint4AddVector3f(GLUSfloat result[4], const GLUSfloat point[4], const GLUSfloat vector[3]);

/**
 * Adds a vector to a 2D point, given as homogeneous coordinates. Result is the new point.
 *
 * @param result The resulting point.
 * @param point The point.
 * @param vector The vector, which is added to the point.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint3AddVector2f(GLUSfloat result[3], const GLUSfloat point[3], const GLUSfloat vector[2]);

/**
 * Subtracts a vector from a 3D point, given as homogeneous coordinates. Result is the new point.
 *
 * @param result The resulting point.
 * @param point The point.
 * @param vector The vector, which is subtracted from the point.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint4SubtractVector3f(GLUSfloat result[4], const GLUSfloat point[4], const GLUSfloat vector[3]);

/**
 * Subtracts a vector from a 2D point, given as homogeneous coordinates. Result is the new point.
 *
 * @param result The resulting point.
 * @param point The point.
 * @param vector The vector, which is subtracted from the point.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint3SubtractVector2f(GLUSfloat result[3], const GLUSfloat point[3], const GLUSfloat vector[2]);

/**
 * Converts a 3D point, given as homogeneous coordinates, to a quaternion.
 *
 * @param result The resulting quaternion.
 * @param point The point, which is converted.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint4GetQuaternionf(GLUSfloat result[4], const GLUSfloat point[4]);

/**
 * Converts a 3D point, given as homogeneous coordinates, to a vector.
 *
 * @param result The resulting vector.
 * @param point The point, which is converted.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint4GetVector3f(GLUSfloat result[3], const GLUSfloat point[4]);

/**
 * Converts a 2D point, given as homogeneous coordinates, to a vector.
 *
 * @param result The resulting vector.
 * @param point The point, which is converted.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPoint3GetVector2f(GLUSfloat result[2], const GLUSfloat point[3]);

/**
 * Calculates the distance of two 3D points, given as homogeneous coordinates.
 *
 * @param point0 The first point.
 * @param point1 The second point.
 *
 * @return The distance of the two points.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusPoint4Distancef(const GLUSfloat point0[4], const GLUSfloat point1[4]);

/**
 * Calculates the distance of two 2D points, given as homogeneous coordinates.
 *
 * @param point0 The first point.
 * @param point1 The second point.
 *
 * @return The distance of the two points.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusPoint3Distancef(const GLUSfloat point0[3], const GLUSfloat point1[3]);

//
// Plane functions.
//

/**
 * Copies a plane.
 *
 * @param result Destination plane.
 * @param plane Source plane.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPlaneCopyf(GLUSfloat result[4], const GLUSfloat plane[4]);

/**
 * Creates a plane. The formula is Ax + By + Cz + D = 0, where A,B,C and D are stored in the resulting array.
 * @see Mathematics For 3D Game Programming & Computer Graphics, Second Edition, Page105
 *
 * @param result The calculated plane.
 * @param point Any point on the plane.
 * @param normal The normal vector of the plane. The vector will be normalized during calculation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusPlaneCreatef(GLUSfloat result[4], const GLUSfloat point[4], const GLUSfloat normal[3]);

/**
 * Calculates the signed distance from a plane to a point. If the value is positive, the point is on the side the normal is directing to.
 *
 * @param plane The used plane.
 * @param point The used point.
 *
 * @return The signed distance.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusPlaneDistancePoint4f(const GLUSfloat plane[4], const GLUSfloat point[4]);

//
// Math functions
//

/**
 * Calculates the maximum of two values.
 *
 * @param value0 The first value.
 * @param value1 The second value.
 *
 * @return The maximum of the two values.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMaxf(const GLUSfloat value0, const GLUSfloat value1);

/**
 * Calculates the minimum of two values.
 *
 * @param value0 The first value.
 * @param value1 The second value.
 *
 * @return The minimum of the two values.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMinf(const GLUSfloat value0, const GLUSfloat value1);

/**
 * Converts radians to degrees.
 *
 * @param radians The angle in radians.
 *
 * @return The angle in degrees.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusRadToDegf(const GLUSfloat radians);

/**
 * Converts degrees to radians.
 *
 * @param degrees The angle in degrees.
 *
 * @return The angle in radians.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusDegToRadf(const GLUSfloat degrees);

/**
 * Linear interpolation of two values. If t = 0, value0 is returned. If t = 1, value1 is returned.
 *
 * @param value0 First value.
 * @param value1 Second value.
 * @param t The fraction.
 *
 * @return The interpolated value.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMixf(const GLUSfloat value0, const GLUSfloat value1, const GLUSfloat t);

/**
 * Clamps a given value between a minimum and maximum value. If the value is between min and max, the value is returned.
 *
 * @param value The value to be clamped.
 * @param min The minimum border. If the value is lower than min, min is returned.
 * @param max The maximum border. If the value is greater than max, max is returned.
 *
 * @return The clamped value.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusClampf(const GLUSfloat value, const GLUSfloat min, const GLUSfloat max);

/**
 * Calculates the length of a vector or the distance of a point to the origin by providing the x, y and z coordinates.
 *
 * @param x The x coordinate.
 * @param y The y coordinate.
 * @param z The z coordinate.
 *
 * @return The calculated length.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusLengthf(const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

//
// View, projection etc. functions.
//

/**
 * Creates a matrix with orthogonal projection.
 * @see http://en.wikipedia.org/wiki/Orthographic_projection_%28geometry%29
 *
 * @param result The resulting matrix.
 * @param left Left corner.
 * @param right Right corner.
 * @param bottom Bottom corner.
 * @param top Top corner.
 * @param nearVal Near corner.
 * @param farVal Far corner.
 *
 * @return GLUS_TRUE, if creation was successful.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusOrthof(GLUSfloat result[16], const GLUSfloat left, const GLUSfloat right, const GLUSfloat bottom, const GLUSfloat top, const GLUSfloat nearVal, const GLUSfloat farVal);

/**
 * Creates a matrix with perspective projection.
 *
 * @param result The resulting matrix.
 * @param left Left corner.
 * @param right Right corner.
 * @param bottom Bottom corner.
 * @param top Top corner.
 * @param nearVal Near corner.
 * @param farVal Far corner.
 *
 * @return GLUS_TRUE, if creation was successful.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusFrustumf(GLUSfloat result[16], const GLUSfloat left, const GLUSfloat right, const GLUSfloat bottom, const GLUSfloat top, const GLUSfloat nearVal, const GLUSfloat farVal);

/**
 * Creates a matrix with perspective projection.
 *
 * @param result The resulting matrix.
 * @param fovy Field of view.
 * @param aspect Aspect ratio.
 * @param zNear Near plane.
 * @param zFar Far plane.
 *
 * @return GLUS_TRUE, if creation was successful.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusPerspectivef(GLUSfloat result[16], const GLUSfloat fovy, const GLUSfloat aspect, const GLUSfloat zNear, const GLUSfloat zFar);

/**
 * Creates a view / camera matrix.
 *
 * @param result The resulting matrix.
 * @param eyeX Eye / camera X position.
 * @param eyeY Eye / camera Y position.
 * @param eyeZ Eye / camera Z position.
 * @param centerX X Position, where the view / camera points to.
 * @param centerY Y Position, where the view / camera points to.
 * @param centerZ Z Position, where the view / camera points to.
 * @param upX Eye / camera X component from up vector.
 * @param upY Eye / camera Y component from up vector.
 * @param upZ Eye / camera Z component from up vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusLookAtf(GLUSfloat result[16], const GLUSfloat eyeX, const GLUSfloat eyeY, const GLUSfloat eyeZ, const GLUSfloat centerX, const GLUSfloat centerY, const GLUSfloat centerZ, const GLUSfloat upX, const GLUSfloat upY, const GLUSfloat upZ);

//
// Matrix functions.
//

/**
 * Sets the given 4x4 matrix to an identity matrix.
 *
 * @param matrix The matrix, which is set to the identity matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Identityf(GLUSfloat matrix[16]);

/**
 * Sets the given 3x3 matrix to an identity matrix.
 *
 * @param matrix The matrix, which is set to the identity matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Identityf(GLUSfloat matrix[9]);

/**
 * Sets the given 2x2 matrix to an identity matrix.
 *
 * @param matrix The matrix, which is set to the identity matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Identityf(GLUSfloat matrix[4]);

/**
 * Copies a 4x4 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 * @param rotationOnly If GLUS_TRUE, only the 3x3 Matrix is copied into an identity 4x4 matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Copyf(GLUSfloat matrix[16], const GLUSfloat source[16], const GLUSboolean rotationOnly);

/**
 * Copies a 3x3 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 * @param rotationOnly If GLUS_TRUE, only the 2x2 Matrix is copied into an identity 3x3 matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Copyf(GLUSfloat matrix[9], const GLUSfloat source[9], const GLUSboolean rotationOnly);

/**
 * Copies a 3x3 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Copyf(GLUSfloat matrix[4], const GLUSfloat source[4]);

/**
 * Extracts a 3x3 matrix out of a 4x4 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4ExtractMatrix3x3f(GLUSfloat matrix[9], const GLUSfloat source[16]);

/**
 * Extracts a 2x2 matrix out of a 4x4 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4ExtractMatrix2x2f(GLUSfloat matrix[4], const GLUSfloat source[16]);

/**
 * Extracts a 2x2 matrix out of a 3x3 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3ExtractMatrix2x2f(GLUSfloat matrix[4], const GLUSfloat source[9]);

/**
 * Creates a 4x4 matrix out of a 3x3 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3CreateMatrix4x4f(GLUSfloat matrix[16], const GLUSfloat source[9]);

/**
 * Creates a 4x4 matrix out of a 2x2 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2CreateMatrix4x4f(GLUSfloat matrix[16], const GLUSfloat source[4]);

/**
 * Creates a 3x3 matrix out of a 2x2 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2CreateMatrix3x3f(GLUSfloat matrix[9], const GLUSfloat source[4]);

/**
 * Multiplies two 4x4 matrices: matrix0 * matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Multiplyf(GLUSfloat matrix[16], const GLUSfloat matrix0[16], const GLUSfloat matrix1[16]);

/**
 * Multiplies two 3x3 matrices: matrix0 * matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Multiplyf(GLUSfloat matrix[9], const GLUSfloat matrix0[9], const GLUSfloat matrix1[9]);

/**
 * Multiplies two 2x2 matrices: matrix0 * matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Multiplyf(GLUSfloat matrix[4], const GLUSfloat matrix0[4], const GLUSfloat matrix1[4]);

/**
 * Adds two 4x4 matrices: matrix0 + matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Addf(GLUSfloat matrix[16], const GLUSfloat matrix0[16], const GLUSfloat matrix1[16]);

/**
 * Adds two 3x3 matrices: matrix0 + matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Addf(GLUSfloat matrix[9], const GLUSfloat matrix0[9], const GLUSfloat matrix1[9]);

/**
 * Adds two 2x2 matrices: matrix0 + matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Addf(GLUSfloat matrix[4], const GLUSfloat matrix0[4], const GLUSfloat matrix1[4]);

/**
 * Subtracts two 4x4 matrices: matrix0 - matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Subtractf(GLUSfloat matrix[16], const GLUSfloat matrix0[16], const GLUSfloat matrix1[16]);

/**
 * Subtracts two 3x3 matrices: matrix0 - matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Subtractf(GLUSfloat matrix[9], const GLUSfloat matrix0[9], const GLUSfloat matrix1[9]);

/**
 * Subtracts two 2x2 matrices: matrix0 - matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Subtractf(GLUSfloat matrix[4], const GLUSfloat matrix0[4], const GLUSfloat matrix1[4]);

/**
 * Calculates the determinant of a 4x4 matrix.
 *
 * @param matrix The used matrix.
 *
 * @return The calculated determinant.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMatrix4x4Determinantf(const GLUSfloat matrix[16]);

/**
 * Calculates the determinant of a 3x3 matrix.
 *
 * @param matrix The used matrix.
 *
 * @return The calculated determinant.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMatrix3x3Determinantf(const GLUSfloat matrix[9]);

/**
 * Calculates the determinant of a 2x2 matrix.
 *
 * @param matrix The used matrix.
 *
 * @return The calculated determinant.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMatrix2x2Determinantf(const GLUSfloat matrix[4]);

/**
 * Calculates the inverse of a 4x4 matrix using Gaussian Elimination.
 *
 * @param matrix The matrix to be inverted.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusMatrix4x4Inversef(GLUSfloat matrix[16]);

/**
 * Calculates the inverse of a 3x3 matrix using the determinant and adjunct of a matrix.
 *
 * @param matrix The matrix to be inverted.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusMatrix3x3Inversef(GLUSfloat matrix[9]);

/**
 * Calculates the inverse of a 2x2 matrix using the determinant and adjunct of a matrix.
 *
 * @param matrix The matrix to be inverted.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusMatrix2x2Inversef(GLUSfloat matrix[4]);

/**
 * Calculates the inverse of a 4x4 matrix by assuming it is a rigid body matrix.
 *
 * @param matrix The matrix to be inverted.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4InverseRigidBodyf(GLUSfloat matrix[16]);

/**
 * Calculates the inverse of a 3x3 matrix by assuming it is a rigid body matrix.
 *
 * @param matrix The matrix to be inverted.
 * @param is2D Set to GLUS_TRUE, if this matrix should be treated as a 2D matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3InverseRigidBodyf(GLUSfloat matrix[9], const GLUSboolean is2D);

/**
 * Calculates the inverse of a 2x2 matrix by assuming it is a rigid body matrix.
 *
 * @param matrix The matrix to be inverted.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2InverseRigidBodyf(GLUSfloat matrix[4]);

/**
 * Transposes a 4x4 matrix.
 *
 * @param matrix The matrix to be transposed.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Transposef(GLUSfloat matrix[16]);

/**
 * Transposes a 3x3 matrix.
 *
 * @param matrix The matrix to be transposed.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Transposef(GLUSfloat matrix[9]);

/**
 * Transposes a 2x2 matrix.
 *
 * @param matrix The matrix to be transposed.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Transposef(GLUSfloat matrix[4]);

/**
 * Creates a 4x4 translation matrix and multiplies it with the provided one: matrix = matrix * translateMatrix
 *
 * @param matrix The final matrix.
 * @param x Amount to translate in x direction.
 * @param y Amount to translate in y direction.
 * @param z Amount to translate in z direction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Translatef(GLUSfloat matrix[16], const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a 3x3 translation matrix and multiplies it with the provided one: matrix = matrix * translateMatrix
 *
 * @param matrix The final matrix.
 * @param x Amount to translate in x direction.
 * @param y Amount to translate in y direction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Translatef(GLUSfloat matrix[9], const GLUSfloat x, const GLUSfloat y);

/**
 * Creates a 4x4 matrix for rotation around a given axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 * @param x X coordinate of axis.
 * @param y Y coordinate of axis.
 * @param z Z coordinate of axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Rotatef(GLUSfloat matrix[16], const GLUSfloat angle, const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a 3x3 matrix for rotation around a given axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 * @param x X coordinate of axis.
 * @param y Y coordinate of axis.
 * @param z Z coordinate of axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Rotatef(GLUSfloat matrix[9], const GLUSfloat angle, const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a 2x2 matrix for a rotation around in 2D and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Rotatef(GLUSfloat matrix[4], const GLUSfloat angle);

/**
 * Creates a 4x4 matrix for rotation around the x axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRxf(GLUSfloat matrix[16], const GLUSfloat angle);

/**
 * Creates a 3x3 matrix for rotation around the x axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRxf(GLUSfloat matrix[9], const GLUSfloat angle);

/**
 * Creates a 4x4 matrix for rotation around the y axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRyf(GLUSfloat matrix[16], const GLUSfloat angle);

/**
 * Creates a 3x3 matrix for rotation around the y axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRyf(GLUSfloat matrix[9], const GLUSfloat angle);

/**
 * Creates a 4x4 matrix for rotation around the z axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRzf(GLUSfloat matrix[16], const GLUSfloat angle);

/**
 * Creates a 3x3 matrix for rotation around the z axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRzf(GLUSfloat matrix[9], const GLUSfloat angle);

/**
 * Creates a 4x4 matrix for rotation using Euler angles and multiplies it with the given one: matrix = matrix * Rz * Rx * Ry
 *
 * @param matrix The final matrix.
 * @param anglez The rotation angle for the z axis in degree.
 * @param anglex The rotation angle for the x axis in degree.
 * @param angley The rotation angle for the y axis in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRzRxRyf(GLUSfloat matrix[16], const GLUSfloat anglez, const GLUSfloat anglex, const GLUSfloat angley);

/**
 * Creates a 4x4 matrix for rotation using Euler angles and multiplies it with the given one: matrix = matrix * Rz * Ry * Rx
 *
 * @param matrix The final matrix.
 * @param anglez The rotation angle for the z axis in degree.
 * @param angley The rotation angle for the y axis in degree.
 * @param anglex The rotation angle for the x axis in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRzRyRxf(GLUSfloat matrix[16], const GLUSfloat anglez, const GLUSfloat angley, const GLUSfloat anglex);

/**
 * Creates a 3x3 matrix for rotation using Euler angles and multiplies it with the given one: matrix = matrix * Rz * Rx * Ry
 *
 * @param matrix The final matrix.
 * @param anglez The rotation angle for the z axis in degree.
 * @param anglex The rotation angle for the x axis in degree.
 * @param angley The rotation angle for the y axis in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRzRxRyf(GLUSfloat matrix[9], const GLUSfloat anglez, const GLUSfloat anglex, const GLUSfloat angley);

/**
 * Creates a 3x3 matrix for rotation using Euler angles and multiplies it with the given one: matrix = matrix * Rz * Ry * Rx
 *
 * @param matrix The final matrix.
 * @param anglez The rotation angle for the z axis in degree.
 * @param angley The rotation angle for the y axis in degree.
 * @param anglex The rotation angle for the x axis in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRzRyRxf(GLUSfloat matrix[9], const GLUSfloat anglez, const GLUSfloat angley, const GLUSfloat anglex);

/**
 * Creates a 4x4 matrix for scaling and multiplies it with the given one: matrix = matrix * scaleMatrix
 *
 * @param matrix The final matrix.
 * @param x The value to scale in the x axis.
 * @param y The value to scale in the y axis.
 * @param z The value to scale in the z axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Scalef(GLUSfloat matrix[16], const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a 3x3 matrix for scaling and multiplies it with the given one: matrix = matrix * scaleMatrix
 *
 * @param matrix The final matrix.
 * @param x The value to scale in the x axis.
 * @param y The value to scale in the y axis.
 * @param z The value to scale in the z axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Scalef(GLUSfloat matrix[9], const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a 2x2 matrix for scaling and multiplies it with the given one: matrix = matrix * scaleMatrix
 *
 * @param matrix The final matrix.
 * @param x The value to scale in the x axis.
 * @param y The value to scale in the y axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Scalef(GLUSfloat matrix[4], const GLUSfloat x, const GLUSfloat y);

/**
 * Creates a 4x4 matrix for shearing and multiplies it with the given one: matrix = matrix * shearMatrix
 *
 * @param matrix The final matrix.
 * @param shxy Value for shearing x in y direction.
 * @param shxz Value for shearing x in z direction.
 * @param shyx Value for shearing y in x direction.
 * @param shyz Value for shearing y in z direction.
 * @param shzx Value for shearing z in x direction.
 * @param shzy Value for shearing z in y direction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Shearf(GLUSfloat matrix[16], const GLUSfloat shxy, const GLUSfloat shxz, const GLUSfloat shyx, const GLUSfloat shyz, const GLUSfloat shzx, const GLUSfloat shzy);

/**
 * Creates a 3x3 matrix for shearing and multiplies it with the given one: matrix = matrix * shearMatrix
 *
 * @param matrix The final matrix.
 * @param shxy Value for shearing x in y direction.
 * @param shxz Value for shearing x in z direction.
 * @param shyx Value for shearing y in x direction.
 * @param shyz Value for shearing y in z direction.
 * @param shzx Value for shearing z in x direction.
 * @param shzy Value for shearing z in y direction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Shearf(GLUSfloat matrix[9], const GLUSfloat shxy, const GLUSfloat shxz, const GLUSfloat shyx, const GLUSfloat shyz, const GLUSfloat shzx, const GLUSfloat shzy);

/**
 * Creates a 2x2 matrix for shearing and multiplies it with the given one: matrix = matrix * shearMatrix
 *
 * @param matrix The final matrix.
 * @param shx Value for shearing in x direction.
 * @param shy Value for shearing in y direction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Shearf(GLUSfloat matrix[4], const GLUSfloat shx, const GLUSfloat shy);

/**
 * Multiplies a 4x4 matrix with a 3D Vector.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyVector3f(GLUSfloat result[3], const GLUSfloat matrix[16], const GLUSfloat vector[3]);

/**
 * Multiplies a 4x4 matrix with a 2D Vector.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyVector2f(GLUSfloat result[2], const GLUSfloat matrix[16], const GLUSfloat vector[2]);

/**
 * Multiplies a 3x3 matrix with a 3D Vector.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3MultiplyVector3f(GLUSfloat result[3], const GLUSfloat matrix[9], const GLUSfloat vector[3]);

/**
 * Multiplies a 3x3 matrix with a 2D Vector.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3MultiplyVector2f(GLUSfloat result[2], const GLUSfloat matrix[9], const GLUSfloat vector[2]);

/**
 * Multiplies a 2x2 matrix with a 2D Vector.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2MultiplyVector2f(GLUSfloat result[2], const GLUSfloat matrix[4], const GLUSfloat vector[2]);

/**
 * Multiplies a 4x4 matrix with a 3D Point, given as homogeneous coordinates.
 *
 * @param result The transformed point.
 * @param matrix The matrix used for the transformation.
 * @param point The used point for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyPoint4f(GLUSfloat result[4], const GLUSfloat matrix[16], const GLUSfloat point[4]);

/**
 * Multiplies a 3x3 matrix with a 2D Point, given as homogeneous coordinates.
 *
 * @param result The transformed point.
 * @param matrix The matrix used for the transformation.
 * @param point The used point for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3MultiplyPoint3f(GLUSfloat result[3], const GLUSfloat matrix[9], const GLUSfloat point[3]);

/**
 * Multiplies a 4x4 matrix with a plane.
 *
 * @param result The transformed plaen.
 * @param matrix The matrix used for the transformation.
 * @param plane The used plane for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyPlanef(GLUSfloat result[4], const GLUSfloat matrix[16], const GLUSfloat plane[4]);

/**
 * Extracts the Euler angles from a 4x4 matrix, which was created by a Rz * Rx * Ry transformation.
 *
 * @param angles Resulting anglex (Pitch), angley (Yaw) and anglez (Roll) in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4GetEulerRzRxRyf(GLUSfloat angles[3], const GLUSfloat matrix[16]);

/**
 * Extracts the Euler angles from a 4x4 matrix, which was created by a Rz * Ry * Rz transformation.
 *
 * @param angles Resulting anglex (Pitch), angley (Yaw) and anglez (Roll) in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4GetEulerRzRyRxf(GLUSfloat angles[3], const GLUSfloat matrix[16]);

/**
 * Extracts the Euler angles from a 3x3 matrix, which was created by a Rz * Rx * Ry transformation.
 *
 * @param angles Resulting anglex (Pitch), angley (Yaw) and anglez (Roll) in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3GetEulerRzRxRyf(GLUSfloat angles[3], const GLUSfloat matrix[9]);

/**
 * Extracts the Euler angles from a 3x3 matrix, which was created by a Rz * Ry * Rx transformation.
 *
 * @param angles Resulting anglex (Pitch), angley (Yaw) and anglez (Roll) in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3GetEulerRzRyRxf(GLUSfloat angles[3], const GLUSfloat matrix[9]);

/**
 * Extracts the angle from a 3x3 matrix, which is used in 2D.
 *
 * @param angle Resulting angle in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3GetAnglef(GLUSfloat* angle, const GLUSfloat matrix[9]);

/**
 * Extracts the angle from a 2x2 matrix.
 *
 * @param angle Resulting angle in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2GetAnglef(GLUSfloat* angle, const GLUSfloat matrix[4]);

/**
 * Extracts the scales from a 4x4 matrix.
 *
 * @param scales Resulting sx, sy, sz.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4GetScale(GLUSfloat scales[3], const GLUSfloat matrix[16]);

/**
 * Extracts the scales from a 3x3 matrix.
 *
 * @param scales Resulting sx, sy, sz.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3GetScale(GLUSfloat scales[3], const GLUSfloat matrix[9]);

/**
 * Extracts the scales from a 2x2 matrix.
 *
 * @param scales Resulting sx, sy.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2GetScale(GLUSfloat scales[2], const GLUSfloat matrix[4]);

/**
 * Extracts the translates from a 4x4 matrix.
 *
 * @param translates Resulting x, y and z.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4GetTranslate(GLUSfloat translates[3], const GLUSfloat matrix[16]);

/**
 * Extracts the translates from a 3x3 matrix, which is used for 2D.
 *
 * @param translates Resulting x and y.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3GetTranslate(GLUSfloat translates[2], const GLUSfloat matrix[9]);

//
// Quaternion functions.
//

/**
 * Creates the identity Quaternion.
 *
 * @param quaternion The resulting identity Quaternion.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionIdentityf(GLUSfloat quaternion[4]);

/**
 * Copies a Quaternion.
 *
 * @param result The destination Quaternion.
 * @param quaternion The source Quaternion.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionCopyf(GLUSfloat result[4], const GLUSfloat quaternion[4]);

/**
 * Calculates the norm of a Quaternion.
 *
 * @param quaternion The Quaternion to calculate the norm from.
 *
 * @return The norm of the Quaternion.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusQuaternionNormf(const GLUSfloat quaternion[4]);

/**
 * Normalized a Quaternion.
 *
 * @param quaternion The Quaternion to normalize.
 *
 * @return GLUS_TRUE, if normalization succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusQuaternionNormalizef(GLUSfloat quaternion[4]);

/**
 * Adds two Quaternions.
 *
 * @param result The sum of both Quaternions.
 * @param quaternion0 The first Quaternion.
 * @param quaternion1 The second Quaternion.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionAddQuaternionf(GLUSfloat result[4], const GLUSfloat quaternion0[4], const GLUSfloat quaternion1[4]);

/**
 * Subtracts two Quaternions: quaternion0 - quaternion1
 *
 * @param result The difference of both Quaternions.
 * @param quaternion0 The first Quaternion.
 * @param quaternion1 The second Quaternion.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionSubQuaternionf(GLUSfloat result[4], const GLUSfloat quaternion0[4], const GLUSfloat quaternion1[4]);

/**
 * Multiplies two Quaternions: quaternion0 * quaternion1
 *
 * @param result The multiplied Quaternion.
 * @param quaternion0 The first Quaternion.
 * @param quaternion1 The second Quaternion.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionMultiplyQuaternionf(GLUSfloat result[4], const GLUSfloat quaternion0[4], const GLUSfloat quaternion1[4]);

/**
 * Calculates the conjugate of a Quaternion.
 *
 * @param quaternion The Quaternion to conjugate.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionConjugatef(GLUSfloat quaternion[4]);

/**
 * Calculates the inverse of a Quaternion.
 *
 * @param quaternion The Quaternion to invert.
 *
 * @return GLUS_TRUE, if the inverse could be calculated.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusQuaternionInversef(GLUSfloat quaternion[4]);

/**
 * Calculates the inverse of a unit Quaternion.
 *
 * @param quaternion The Quaternion to invert.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionInverseUnitf(GLUSfloat quaternion[4]);

/**
 * Creates a rotation Quaternion around a given axis.
 *
 * @param quaternion The final quaternion.
 * @param angle The rotation angle in degree.
 * @param x The x coordinate of the axis.
 * @param y The y coordinate of the axis.
 * @param z The z coordinate of the axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionRotatef(GLUSfloat quaternion[4], const GLUSfloat angle, const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a rotation Quaternion around a the x axis.
 *
 * @param quaternion The final quaternion.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionRotateRxf(GLUSfloat quaternion[4], const GLUSfloat angle);

/**
 * Creates a rotation Quaternion around a the y axis.
 *
 * @param quaternion The final quaternion.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionRotateRyf(GLUSfloat quaternion[4], const GLUSfloat angle);

/**
 * Creates a rotation Quaternion around a the z axis.
 *
 * @param quaternion The final quaternion.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionRotateRzf(GLUSfloat quaternion[4], const GLUSfloat angle);

/**
 * Creates a rotation Quaternion out of Euler angels: Rz * Rx * Ry
 *
 * @param quaternion The final quaternion.
 * @param anglez The rotation z angle in degree.
 * @param anglex The rotation x angle in degree.
 * @param angley The rotation y angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionRotateRzRxRyf(GLUSfloat quaternion[4], const GLUSfloat anglez, const GLUSfloat anglex, const GLUSfloat angley);

/**
 * Creates a rotation Quaternion out of Euler angels: Rz * Ry * Rx
 *
 * @param quaternion The final quaternion.
 * @param anglez The rotation z angle in degree.
 * @param angley The rotation y angle in degree.
 * @param anglex The rotation x angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionRotateRzRyRxf(GLUSfloat quaternion[4], const GLUSfloat anglez, const GLUSfloat angley, const GLUSfloat anglex);

/**
 * Creates a 4x4 matrix out of a Quaternion.
 *
 * @param matrix The resulting matrix.
 * @param quaternion The final quaternion.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionGetMatrix4x4f(GLUSfloat matrix[16], const GLUSfloat quaternion[4]);

/**
 * Creates a 3x3 matrix out of a Quaternion.
 *
 * @param matrix The resulting matrix.
 * @param quaternion The final Quaternion.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionGetMatrix3x3f(GLUSfloat matrix[9], const GLUSfloat quaternion[4]);

/**
 * Creates a 3D Point, given as homogeneous coordinates, out of a Quaternion.
 *
 * @param point The resulting point.
 * @param quaternion The used Quaternion.
 *
 * @return GLUS_TRUE, if a point could be created.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusQuaternionGetPoint4f(GLUSfloat point[4], const GLUSfloat quaternion[4]);

/**
 * Creates a 3D Vector, out of a Quaternion.
 *
 * @param vector The resulting vecotor.
 * @param quaternion The used Quaternion.
 *
 * @return GLUS_TRUE, if a vector could be created.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusQuaternionGetVector3f(GLUSfloat vector[3], const GLUSfloat quaternion[4]);

/**
 * Extracts the Euler angles from a Quaternion, created by Rz * Rx * Ry.
 *
 * @param angles Resulting anglex (Pitch), angley (Yaw) and anglez (Roll) in degrees.
 * @param quaternion The used Quaternion.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionGetEulerRzRxRyf(GLUSfloat angles[3], const GLUSfloat quaternion[4]);

/**
 * Extracts the Euler angles from a Quaternion, created by Rz * Ry * Rx.
 *
 * @param angles Resulting anglex (Pitch), angley (Yaw) and anglez (Roll) in degrees.
 * @param quaternion The used Quaternion.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionGetEulerRzRyRxf(GLUSfloat angles[3], const GLUSfloat quaternion[4]);

/**
 * Spherical interpolation of two Quaternions.
 *
 * @param result The interpolated Quaternion.
 * @param quaternion0 The first Quaternion.
 * @param quaternion1 The second Quaternion.
 * @param t The fraction of both Quaternions.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionSlerpf(GLUSfloat result[4], const GLUSfloat quaternion0[4], const GLUSfloat quaternion1[4], const GLUSfloat t);

//
// Shader creation function.
//

/**
 * Creates a program by compiling the giving sources. Linking has to be done in a separate step.
 *
 * @param shaderProgram This structure holds the necessary information of the program and the different shaders.
 * @param vertexSource Vertex shader source code.
 * @param fragmentSource Fragment shader source code.
 *
 * @return GLUS_TRUE, if compiling and creation of program succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateProgramFromSource(GLUSshaderprogram* shaderProgram, const GLUSchar** vertexSource, const GLUSchar** fragmentSource);

/**
 * Links a formerly created program.
 *
 * @param shaderProgram This structure holds the necessary information of the program and the different shaders.
 *
 * @return GLUS_TRUE, if linking of program succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLinkProgram(GLUSshaderprogram* shaderProgram);

/**
 * Builds a program by compiling and linking the giving sources.
 *
 * @param shaderProgram This structure holds the necessary information of the program and the different shaders.
 * @param vertexSource Vertex shader source code.
 * @param fragmentSource Fragment shader source code.
 *
 * @return GLUS_TRUE, if compiling and linking of program succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusBuildProgramFromSource(GLUSshaderprogram* shaderProgram, const GLUSchar** vertexSource, const GLUSchar** fragmentSource);

/**
 * Destroys a program by freeing all resources.
 *
 * @param shaderprogram This structure holds the necessary information of the program and the different shaders.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyProgram(GLUSshaderprogram* shaderprogram);

//
// Shape / geometry functions.
//

/**
 * Creates a quadratic plane.
 *
 * @param shape The data is stored into this structure.
 * @param halfExtend The length from the center point to any border of the plane.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreatePlanef(GLUSshape* shape, const GLUSfloat halfExtend);

/**
 * Creates a rectangular plane.
 *
 * @param shape The data is stored into this structure.
 * @param horizontalExtend The length from the center point to the left/right border of the plane.
 * @param verticalExtend The length from the center point to the upper/lower border of the plane.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateRectangularPlanef(GLUSshape* shape, const GLUSfloat horizontalExtend, const GLUSfloat verticalExtend);

/**
 * Creates a rectangular plane organized as a grid with given number rows and columns.
 *
 * @param shape The data is stored into this structure.
 * @param horizontalExtend The length from the center point to the left/right border of the plane.
 * @param verticalExtend The length from the center point to the upper/lower border of the plane.
 * @param rows The number of rows the grid should have.
 * @param columns The number of columns the grid should have.
 * @param triangleStrip Set to GLUS_TRUE, if a triangle strip should be created.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateRectangularGridPlanef(GLUSshape* shape, const GLUSfloat horizontalExtend, const GLUSfloat verticalExtend, const GLUSushort rows, const GLUSushort columns, const GLUSboolean triangleStrip);

/**
 * Creates a disc with the given radius and number sectors. More sectors makes the disc more round.
 *
 * @param shape The data is stored into this structure.
 * @param radius The radius of the disc.
 * @param numberSectors The number of sectors the disc should have.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateDiscf(GLUSshape* shape, const GLUSfloat radius, const GLUSushort numberSectors);

/**
 * Creates a cube.
 *
 * @param shape The data is stored into this structure.
 * @param halfExtend The distance from the center point to any face of the cube.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateCubef(GLUSshape* shape, const GLUSfloat halfExtend);

/**
 * Creates a sphere. More slices makes the sphere more round.
 *
 * @param shape The data is stored into this structure.
 * @param radius The radius of the sphere.
 * @param numberSlices The number of slices the sphere should have.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateSpheref(GLUSshape* shape, const GLUSfloat radius, const GLUSushort numberSlices);

/**
 * Creates a dome. More slices makes the dome more round.
 *
 * @param shape The data is stored into this structure.
 * @param radius The radius of the dome.
 * @param numberSlices The number of slices the dome should have.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateDomef(GLUSshape* shape, const GLUSfloat radius, const GLUSushort numberSlices);

/**
 * Creates a torus / doughnut. More slices and stacks makes the torus more round.
 *
 * @param shape The data is stored into this structure.
 * @param innerRadius The inner radius of the torus. This is the distance from the center point to the closest point from the torus surface.
 * @param outerRadius The outer radius of the torus. This is the distance from the center point to the farthest point from the torus surface.
 * @param numberSlices The number of slices the torus should have.
 * @param numberStacks The number of stacks / elements the torus should have per slice.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateTorusf(GLUSshape* shape, const GLUSfloat innerRadius, const GLUSfloat outerRadius, const GLUSushort numberSlices, const GLUSushort numberStacks);

/**
 * Creates a cylinder. More slices makes the cylinder more round.
 *
 * @param shape The data is stored into this structure.
 * @param halfExtend The distance from the center point to the bottom and top disc of the cylinder.
 * @param radius The radius of the cylinder.
 * @param numberSlices The number of slices the cylinder should have.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateCylinderf(GLUSshape* shape, const GLUSfloat halfExtend, const GLUSfloat radius, const GLUSushort numberSlices);

/**
 * Creates a cone. More slices and stacks makes the torus more round and smooth.
 *
 * @param shape The data is stored into this structure.
 * @param halfExtend The distance from the center point to the bottom disc of the cone.
 * @param radius The radius of the cone at the bottom.
 * @param numberSlices The number of slices the cone should have.
 * @param numberStacks The number of stacks the cone should have.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateConef(GLUSshape* shape, const GLUSfloat halfExtend, const GLUSfloat radius, const GLUSushort numberSlices, const GLUSushort numberStacks);

/**
 * Calculates and creates the tangent and bitangent vectors. Uses the previous created memory for the tangents and bitangents.
 *
 * @param shape The structure which will be filled with the calculated vectors.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCalculateTangentSpacef(GLUSshape* shape);

/**
 * Copies the shape.
 *
 * @param shape  The target shape.
 * @param source The source shape.
 *
 * @return GLUS_TRUE, if copy succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCopyShapef(GLUSshape* shape, const GLUSshape* source);

/**
 * Destroys the shape by freeing the allocated memory.
 *
 * @param shape The structure which contains the dynamic allocated shape data, which will be freed by this function.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyShapef(GLUSshape* shape);

//
// Line / geometry functions.
//

/**
 * Creates a line out of two points.
 *
 * @param line The data is stored into this structure.
 * @param point0 The starting point.
 * @param point1 The ending point.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateLinef(GLUSline* line, const GLUSfloat point0[4], const GLUSfloat point1[4]);

/**
 * Creates a square out of lines.
 *
 * @param line The data is stored into this structure.
 * @param halfExtend The length from the center point to edge of the square.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateSquaref(GLUSline* line, const GLUSfloat halfExtend);

/**
 * Creates a rectangular grid out of lines. The grid is centered.
 *
 * @param line The data is stored into this structure.
 * @param horizontalExtend The width of the grid.
 * @param verticalExtend The height of the grid.
 * @param rows Number of rows of the grid.
 * @param columns Number of columns of the grid.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateRectangularGridf(GLUSline* line, const GLUSfloat horizontalExtend, const GLUSfloat verticalExtend, const GLUSuint rows, const GLUSuint columns);

/**
 * Creates a circle out of lines with the given radius and number sectors. More sectors makes the circle more round.
 *
 * @param line The data is stored into this structure.
 * @param radius The radius of the circle.
 * @param numberSectors The number of sectors the circle should have.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateCirclef(GLUSline* line, const GLUSfloat radius, const GLUSushort numberSectors);

/**
 * Copies the line.
 *
 * @param line The target line.
 * @param source The source line.
 *
 * @return GLUS_TRUE, if copy succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCopyLinef(GLUSline* line, const GLUSline* source);

/**
 * Destroys the line by freeing the allocated memory.
 *
 * @param line The structure which contains the dynamic allocated line data, which will be freed by this function.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyLinef(GLUSline* line);

//
// Model loading functions.
//

/**
 * Loads a wavefront object file.
 *
 * @param filename The name of the wavefront file including extension.
 * @param shape The data is stored into this structure.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLoadObjFile(const GLUSchar* filename, GLUSshape* shape);

//
// Logging
//

/**
 * Sets the current log level.
 *
 * @param verbosity The new log level.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusLogSetLevel(const GLUSuint verbosity);

/**
 * Gets the current log level.
 *
 * @return The current log level.
 */
GLUSAPI GLUSuint GLUSAPIENTRY glusLogGetLevel();

/**
 * Prints out the given information to the log console.
 *
 * @param verbosity The used log level.
 * @param format String to log plus format information.
 * @param ... Parameter list for the format string.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusLogPrint(GLUSuint verbosity, const char* format, ...);

//
// GLFW Start. Internally, some GLFW functions are used. See copyright informations in C file.
//

/* Keyboard key definitions: 8-bit ISO-8859-1 (Latin 1) encoding is used
 * for printable keys (such as A-Z, 0-9 etc), and values above 256
 * represent special (non-printable) keys (e.g. F1, Page Up etc).
 */
#define GLFW_KEY_UNKNOWN      -1
#define GLFW_KEY_SPACE        32
#define GLFW_KEY_SPECIAL      256
#define GLFW_KEY_ESC          (GLFW_KEY_SPECIAL+1)
#define GLFW_KEY_F1           (GLFW_KEY_SPECIAL+2)
#define GLFW_KEY_F2           (GLFW_KEY_SPECIAL+3)
#define GLFW_KEY_F3           (GLFW_KEY_SPECIAL+4)
#define GLFW_KEY_F4           (GLFW_KEY_SPECIAL+5)
#define GLFW_KEY_F5           (GLFW_KEY_SPECIAL+6)
#define GLFW_KEY_F6           (GLFW_KEY_SPECIAL+7)
#define GLFW_KEY_F7           (GLFW_KEY_SPECIAL+8)
#define GLFW_KEY_F8           (GLFW_KEY_SPECIAL+9)
#define GLFW_KEY_F9           (GLFW_KEY_SPECIAL+10)
#define GLFW_KEY_F10          (GLFW_KEY_SPECIAL+11)
#define GLFW_KEY_F11          (GLFW_KEY_SPECIAL+12)
#define GLFW_KEY_F12          (GLFW_KEY_SPECIAL+13)
#define GLFW_KEY_F13          (GLFW_KEY_SPECIAL+14)
#define GLFW_KEY_F14          (GLFW_KEY_SPECIAL+15)
#define GLFW_KEY_F15          (GLFW_KEY_SPECIAL+16)
#define GLFW_KEY_F16          (GLFW_KEY_SPECIAL+17)
#define GLFW_KEY_F17          (GLFW_KEY_SPECIAL+18)
#define GLFW_KEY_F18          (GLFW_KEY_SPECIAL+19)
#define GLFW_KEY_F19          (GLFW_KEY_SPECIAL+20)
#define GLFW_KEY_F20          (GLFW_KEY_SPECIAL+21)
#define GLFW_KEY_F21          (GLFW_KEY_SPECIAL+22)
#define GLFW_KEY_F22          (GLFW_KEY_SPECIAL+23)
#define GLFW_KEY_F23          (GLFW_KEY_SPECIAL+24)
#define GLFW_KEY_F24          (GLFW_KEY_SPECIAL+25)
#define GLFW_KEY_F25          (GLFW_KEY_SPECIAL+26)
#define GLFW_KEY_UP           (GLFW_KEY_SPECIAL+27)
#define GLFW_KEY_DOWN         (GLFW_KEY_SPECIAL+28)
#define GLFW_KEY_LEFT         (GLFW_KEY_SPECIAL+29)
#define GLFW_KEY_RIGHT        (GLFW_KEY_SPECIAL+30)
#define GLFW_KEY_LSHIFT       (GLFW_KEY_SPECIAL+31)
#define GLFW_KEY_RSHIFT       (GLFW_KEY_SPECIAL+32)
#define GLFW_KEY_LCTRL        (GLFW_KEY_SPECIAL+33)
#define GLFW_KEY_RCTRL        (GLFW_KEY_SPECIAL+34)
#define GLFW_KEY_LALT         (GLFW_KEY_SPECIAL+35)
#define GLFW_KEY_RALT         (GLFW_KEY_SPECIAL+36)
#define GLFW_KEY_TAB          (GLFW_KEY_SPECIAL+37)
#define GLFW_KEY_ENTER        (GLFW_KEY_SPECIAL+38)
#define GLFW_KEY_BACKSPACE    (GLFW_KEY_SPECIAL+39)
#define GLFW_KEY_INSERT       (GLFW_KEY_SPECIAL+40)
#define GLFW_KEY_DEL          (GLFW_KEY_SPECIAL+41)
#define GLFW_KEY_PAGEUP       (GLFW_KEY_SPECIAL+42)
#define GLFW_KEY_PAGEDOWN     (GLFW_KEY_SPECIAL+43)
#define GLFW_KEY_HOME         (GLFW_KEY_SPECIAL+44)
#define GLFW_KEY_END          (GLFW_KEY_SPECIAL+45)
#define GLFW_KEY_KP_0         (GLFW_KEY_SPECIAL+46)
#define GLFW_KEY_KP_1         (GLFW_KEY_SPECIAL+47)
#define GLFW_KEY_KP_2         (GLFW_KEY_SPECIAL+48)
#define GLFW_KEY_KP_3         (GLFW_KEY_SPECIAL+49)
#define GLFW_KEY_KP_4         (GLFW_KEY_SPECIAL+50)
#define GLFW_KEY_KP_5         (GLFW_KEY_SPECIAL+51)
#define GLFW_KEY_KP_6         (GLFW_KEY_SPECIAL+52)
#define GLFW_KEY_KP_7         (GLFW_KEY_SPECIAL+53)
#define GLFW_KEY_KP_8         (GLFW_KEY_SPECIAL+54)
#define GLFW_KEY_KP_9         (GLFW_KEY_SPECIAL+55)
#define GLFW_KEY_KP_DIVIDE    (GLFW_KEY_SPECIAL+56)
#define GLFW_KEY_KP_MULTIPLY  (GLFW_KEY_SPECIAL+57)
#define GLFW_KEY_KP_SUBTRACT  (GLFW_KEY_SPECIAL+58)
#define GLFW_KEY_KP_ADD       (GLFW_KEY_SPECIAL+59)
#define GLFW_KEY_KP_DECIMAL   (GLFW_KEY_SPECIAL+60)
#define GLFW_KEY_KP_EQUAL     (GLFW_KEY_SPECIAL+61)
#define GLFW_KEY_KP_ENTER     (GLFW_KEY_SPECIAL+62)
#define GLFW_KEY_KP_NUM_LOCK  (GLFW_KEY_SPECIAL+63)
#define GLFW_KEY_CAPS_LOCK    (GLFW_KEY_SPECIAL+64)
#define GLFW_KEY_SCROLL_LOCK  (GLFW_KEY_SPECIAL+65)
#define GLFW_KEY_PAUSE        (GLFW_KEY_SPECIAL+66)
#define GLFW_KEY_LSUPER       (GLFW_KEY_SPECIAL+67)
#define GLFW_KEY_RSUPER       (GLFW_KEY_SPECIAL+68)
#define GLFW_KEY_MENU         (GLFW_KEY_SPECIAL+69)
#define GLFW_KEY_LAST         GLFW_KEY_MENU

/* Key and button state/action definitions */
#define GLFW_RELEASE            0
#define GLFW_PRESS              1

/* Mouse button definitions */
#define GLFW_MOUSE_BUTTON_1      0
#define GLFW_MOUSE_BUTTON_2      1
#define GLFW_MOUSE_BUTTON_3      2
#define GLFW_MOUSE_BUTTON_4      3
#define GLFW_MOUSE_BUTTON_5      4
#define GLFW_MOUSE_BUTTON_6      5
#define GLFW_MOUSE_BUTTON_7      6
#define GLFW_MOUSE_BUTTON_8      7
#define GLFW_MOUSE_BUTTON_LAST   GLFW_MOUSE_BUTTON_8

/* Mouse button aliases */
#define GLFW_MOUSE_BUTTON_LEFT   GLFW_MOUSE_BUTTON_1
#define GLFW_MOUSE_BUTTON_RIGHT  GLFW_MOUSE_BUTTON_2
#define GLFW_MOUSE_BUTTON_MIDDLE GLFW_MOUSE_BUTTON_3

//
// GLFW End
//

#ifdef __cplusplus
}
#endif

#endif /*__glus_h_*/
