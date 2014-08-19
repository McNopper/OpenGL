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

#if (GLUS_ES || GLUS_ES2 || GLUS_ES3 || GLUS_ES31)

#ifdef GLUS_ES31
#include "../GLES3/glus31.h"
#elif GLUS_ES2
#include "../GLES2/glus2.h"
#else
#include "../GLES3/glus3.h"
#endif

#elif (GLUS_VG || GLUS_VG11)  /*GLUS_ES || GLUS_ES2 || GLUS_ES3 || GLUS_ES31*/

#include "../VG/glus.h"

#else /* GLUS_VG || GLUS_VG11 */

#ifndef __glus_h_
#define __glus_h_

#ifdef __cplusplus
extern "C"
{
#endif

//
// GLUS libraries.
//

#include "../GLUS/glus_libs.h"

//
// GLEW for binding OpenGL functions. GLFW for hardware abstraction.
//

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

typedef int EGLint;

#define EGL_TRUE                          1
#define EGL_FALSE                         0
#define EGL_NONE                          0x3038
#define EGL_DONT_CARE                     ((EGLint)-1)

//

#define EGL_RENDERABLE_TYPE               0x3040
#define EGL_OPENGL_BIT                    0x0008

#define EGL_RED_SIZE                      0x3024
#define EGL_GREEN_SIZE                    0x3023
#define EGL_BLUE_SIZE                     0x3022
#define EGL_DEPTH_SIZE                    0x3025
#define EGL_STENCIL_SIZE                  0x3026
#define EGL_ALPHA_SIZE                    0x3021
#define EGL_SAMPLES                       0x3031
#define EGL_SAMPLE_BUFFERS                0x3032

//

#define EGL_CONTEXT_CLIENT_VERSION						0x3098
#define EGL_CONTEXT_MAJOR_VERSION         				0x3098
#define EGL_CONTEXT_MINOR_VERSION         				0x30FB
#define EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE 			0x31B1

#define EGL_CONTEXT_OPENGL_PROFILE_MASK   				0x30FD
#define EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT				0x00000001
#define EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT	0x00000002

#define EGL_CONTEXT_OPENGL_DEBUG          				0x31B0

//
// GLUS function call convention etc.
//

#include "../GLUS/glus_function.h"

//
// GLUS types.
//

#include "../GLUS/glus_types.h"

//
// GLUS defines.
//

#include "../GLUS/glus_define.h"

#include "../GLUS/glus_define_shader.h"

#include "../GLUS/glus_define_color.h"

#include "../GLUS/glus_define_uint.h"

//
// Load and save structures.
//

#include "../GLUS/glus_load_save.h"

//
// Shape with unsigned integer indices.
//

#include "../GLUS/glus_shape_uint.h"

//
// Line with unsigned integer indices.
//

#include "../GLUS/glus_line_uint.h"

//
// Wavefront with unsigned integer indices.
//

#include "../GLUS/glus_wavefront_uint.h"

//
// Memory manager.
//

#include "../GLUS/glus_memory.h"

//
// Callback functions.
//

#include "../GLUS/glus_callback.h"

//
// Window preparation and creation functions.
//

#include "../GLUS/glus_glfw.h"

//
// Version check functions.
//

#include "../GLUS/glus_version.h"

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

#include "../GLUS/glus_shaderprogram.h"

#include "../GLUS/glus_programpipeline.h"

//
// Shape / geometry functions.
//

#include "../GLUS/glus_shape.h"

//
// Shape adjacency
//

#include "../GLUS/glus_shape_adjacency.h"

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

#ifdef __cplusplus
}
#endif

#endif /*__glus_h_*/

#endif /*GLUS_ES*/
