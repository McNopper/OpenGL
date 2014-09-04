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

#ifndef GLUS_DEFINE_H_
#define GLUS_DEFINE_H_

#define GLUS_NO_ERROR 0
#define GLUS_OK 	  0
#define GLUS_TRUE     1
#define GLUS_FALSE    0

#define GLUS_POINTS 									   0x0000

#define GLUS_LINES										   0x0001
#define GLUS_LINE_LOOP									   0x0002
#define GLUS_LINE_STRIP 								   0x0003
#define GLUS_LINES_ADJACENCY							   0x000A
#define GLUS_LINE_STRIP_ADJACENCY						   0x000B

#define GLUS_TRIANGLES									   0x0004
#define GLUS_TRIANGLE_STRIP 							   0x0005
#define GLUS_TRIANGLE_FAN								   0x0006
#define GLUS_TRIANGLES_ADJACENCY						   0x000C
#define GLUS_TRIANGLE_STRIP_ADJACENCY					   0x000D

#define GLUS_UNPACK_ALIGNMENT 							   0x0CF5
#define GLUS_PACK_ALIGNMENT								   0x0D05

#define GLUS_BYTE										   0x1400
#define GLUS_UNSIGNED_BYTE								   0x1401
#define GLUS_SHORT										   0x1402
#define GLUS_UNSIGNED_SHORT								   0x1403
#define GLUS_INT										   0x1404
#define GLUS_UNSIGNED_INT								   0x1405
#define GLUS_FLOAT										   0x1406
#define GLUS_DOUBLE										   0x140A

#define GLUS_VERSION									   0x1F02
#define GLUS_EXTENSIONS									   0x1F03

#define GLUS_COMPILE_STATUS								   0x8B81
#define GLUS_LINK_STATUS								   0x8B82
#define GLUS_VALIDATE_STATUS							   0x8B83
#define GLUS_INFO_LOG_LENGTH							   0x8B84

#define GLUS_FRAMEBUFFER								   0x8D40

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

#define GLUS_MAX_STRING  256

#endif /* GLUS_DEFINE_H_ */
