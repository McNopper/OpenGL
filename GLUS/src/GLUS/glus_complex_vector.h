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

#ifndef GLUS_COMPLEX_VECTOR_H_
#define GLUS_COMPLEX_VECTOR_H_

/**
 * Copies a Vector with N elements.
 *
 * @param result The destination vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVectorNCopyc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Converts a complex number Vector with N elements to a float Vector.
 *
 * @param result The destination vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVectorNConvertToFloatc(GLUSfloat* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Converts a float Vector with N elements to a complex Vector.
 *
 * @param result The destination vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVectorNConvertToComplexc(GLUScomplex* result, const GLUSfloat* vector, const GLUSint n);

/**
 * Multiplies a Vector with N elements by a scalar.
 *
 * @param result The final vector.
 * @param vector The used vector for multiplication.
 * @param n 	 The number of elements.
 * @param scalar The scalar.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVectorNMultiplyScalarc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n, const GLUScomplex scalar);

/**
 * Calculates the complex conjugate of a Vector with N elements.
 *
 * @param result The vector, containing the complex conjugate.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusVectorNConjugatec(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

#endif /* GLUS_COMPLEX_VECTOR_H_ */
