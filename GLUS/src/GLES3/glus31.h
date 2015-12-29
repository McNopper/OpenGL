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

#ifndef __glus_h_
#define __glus_h_

#ifdef __cplusplus
extern "C"
{
#endif

//
// GLUS libraries.
//

#include "../GLUS/glus_clib.h"

//
// OpenGL ES 3.1 and EGL.
//

#define GL_GLEXT_PROTOTYPES 1

#include <GLES3/gl31.h>
#include <EGL/egl.h>

//
// GLUS function call convention etc.
//

#include "../GLUS/glus_function.h"

//
// GLUS types.
//

#include "../GLUS/glus_typedef.h"

//
// GLUS defines.
//

#include "../GLUS/glus_define.h"

#include "../GLUS/glus_define_shader_es31.h"

#include "../GLUS/glus_define_color.h"

#include "../GLUS/glus_define_color_channel_es.h"

#include "../GLUS/glus_define_uint.h"

#define GLUS_EGL_API EGL_OPENGL_ES_API

//
// Image and file structures.
//

#include "../GLUS/glus_file.h"
#include "../GLUS/glus_image.h"

//
// Memory manager.
//

#include "../GLUS/glus_memory.h"

//
// EGL helper functions.
//

#include "../GLUS/glus_egl.h"

//
// Window preparation and creation functions.
//

#include "../GLUS/glus_window.h"

//
// Version check functions.
//

#include "../GLUS/glus_version.h"

//
// Extension functions.
//

#include "../GLUS/glus_extension.h"

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

#include "../GLUS/glus_axisalignedbox.h"

//
// Oriented Box functions.
//

#include "../GLUS/glus_orientedbox.h"

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

#include "../GLUS/glus_matrix_viewprojection.h"

//
// Functions, related to planar surfaces.
//

#include "../GLUS/glus_matrix_planar.h"

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

//
// Fourier functions.
//

#include "../GLUS/glus_fourier.h"

//
// Shader creation function.
//

#include "../GLUS/glus_program_es31.h"

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
#include "../GLUS/glus_line_wavefront.h"

//
// Model loading functions.
//

#include "../GLUS/glus_shape_wavefront.h"
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
// Time
//

#include "../GLUS/glus_time.h"

//
// Ray tracing
//

#include "../GLUS/glus_raytrace.h"

//
// Intersection testing
//

#include "../GLUS/glus_intersect.h"

//
// Textures and files
//

#include "../GLUS/glus_image_tga.h"
#include "../GLUS/glus_image_hdr.h"
#include "../GLUS/glus_image_pkm.h"

#include "../GLUS/glus_file_text.h"
#include "../GLUS/glus_file_binary.h"

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
