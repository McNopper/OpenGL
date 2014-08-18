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

//
// GLUS libraries.
//

#include "../GLUS/glus_libs.h"

//
// OpenGL ES 2.0 and EGL.
//

#include <GLES2/gl2.h>
#include <EGL/egl.h>

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

#include "../GLUS/glus_define_shader_es.h"

#include "../GLUS/glus_define_color_es2.h"

#include "../GLUS/glus_define_ushort.h"

#define GLUS_EGL_API EGL_OPENGL_ES_API

//
// Load and save structures.
//

#include "../GLUS/glus_load_save.h"

//
// Shape with unsigned short indices.
//

#include "../GLUS/glus_shape_ushort.h"

//
// Line with unsigned short indices.
//

#include "../GLUS/glus_line_ushort.h"

//
// Wavefront with unsigned short indices.
//

#include "../GLUS/glus_wavefront_ushort.h"

//
// Memory manager.
//

#include "../GLUS/glus_memory.h"

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

#include "../GLUS/glus_shaderprogram_es.h"

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
