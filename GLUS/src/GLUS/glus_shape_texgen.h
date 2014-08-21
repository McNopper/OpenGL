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

#ifndef GLUS_SHAPE_TEXGEN_H_
#define GLUS_SHAPE_TEXGEN_H_

/**
 * Creates the texture coordinates of a shape. Already existing texture coordinates are deleted.
 *
 * @param shape 	The shape, where the texture coordinates are created.
 * @param sSizeX 	Size of the s texture coordinate, considering x axis.
 * @param sSizeZ 	Size of the s texture coordinate, considering z axis.
 * @param tSizeY 	Size of the t texture coordinate, considering y axis.
 * @param tSizeZ 	Size of the t texture coordinate, considering z axis.
 * @param sOffset 	Offset in the s texture coordinate direction.
 * @param tOffset 	Offset in the t texture coordinate direction.
 *
 * @return GLUS_TURE. if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusShapeTexGenByAxesf(GLUSshape* shape, const GLUSfloat sSizeX, const GLUSfloat sSizeZ, const GLUSfloat tSizeY, const GLUSfloat tSizeZ, const GLUSfloat sOffset, const GLUSfloat tOffset);

/**
 * Creates the texture coordinates of a shape. Already existing texture coordinates are deleted.
 *
 * @param shape 	The shape, where the texture coordinates are created.
 * @param sPlane 	The plane for calculating the s coordinate.
 * @param tPlane 	The plane for calculating the t coordinate.
 * @param sSize 	The size in s direction.
 * @param tSize 	The size in t direction.
 * @param sOffset 	The offset in s direction.
 * @param tOffset 	The offset in t direction.
 *
 * @return GLUS_TURE. if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusShapeTexGenByPlanesf(GLUSshape* shape, const GLUSfloat sPlane[4], const GLUSfloat tPlane[4], const float sSize, const float tSize, const float sOffset, const float tOffset);

#endif /* GLUS_SHAPE_TEXGEN_H_ */
