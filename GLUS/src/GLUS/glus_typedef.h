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

#ifndef GLUS_TYPEDEF_H_
#define GLUS_TYPEDEF_H_

typedef unsigned char GLUSboolean;
typedef signed char GLUSbyte;
typedef unsigned char GLUSubyte;
typedef char GLUSchar;
typedef short GLUSshort;
typedef unsigned short GLUSushort;
typedef int GLUSint;
typedef unsigned int GLUSuint;
typedef int GLUSfixed;
typedef int64_t  GLUSint64;
typedef uint64_t GLUSuint64;
typedef int GLUSsizei;
typedef unsigned int GLUSenum;
// GLUSintptr, GLUSsizeiptr and GLUSsync not implemented.
typedef unsigned int GLUSbitfield;
typedef unsigned short GLUShalf;
typedef float GLUSfloat;
typedef float GLUSclampf;
typedef double GLUSdouble;
typedef double GLUSclampd;

typedef struct _GLUScomplex
{
	GLUSfloat real;
	GLUSfloat imaginary;
} GLUScomplex;

#ifdef __cplusplus
#define GLUSvoid void
#else
typedef void GLUSvoid;
#endif

#endif /* GLUS_TYPEDEF_H_ */
