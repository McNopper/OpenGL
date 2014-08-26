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

#ifndef GLUS_DEFINE_SHADER_ES_H_
#define GLUS_DEFINE_SHADER_ES_H_

#define GLUS_VERTEX_SHADER              0x00008B31
#define GLUS_FRAGMENT_SHADER            0x00008B30
#define GLUS_TESS_EVALUATION_SHADER     0x00008E87
#define GLUS_TESS_CONTROL_SHADER        0x00008E88
#define GLUS_GEOMETRY_SHADER            0x00008DD9
#define GLUS_COMPUTE_SHADER 			0x000091B9

#define GLUS_VERTEX_SHADER_BIT 			0x00000001
#define GLUS_FRAGMENT_SHADER_BIT 		0x00000002
#define GLUS_GEOMETRY_SHADER_BIT 		0x00000004
#define GLUS_TESS_CONTROL_SHADER_BIT 	0x00000008
#define GLUS_TESS_EVALUATION_SHADER_BIT 0x00000010
#define GLUS_COMPUTE_SHADER_BIT 		0x00000020
#define GL_ALL_SHADER_BITS 				0xFFFFFFFF

#endif /* GLUS_DEFINE_SHADER_ES_H_ */
