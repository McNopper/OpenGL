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

#ifndef GLUS_COMPLEX_MATRIX_H_
#define GLUS_COMPLEX_MATRIX_H_

/**
 * Multiplies a NxN matrix with a Vector with N elements.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusMatrixNxNMultiplyVectorNc(GLUScomplex* result, const GLUScomplex* matrix, const GLUScomplex* vector, const GLUSint n);

#endif /* GLUS_COMPLEX_MATRIX_H_ */
