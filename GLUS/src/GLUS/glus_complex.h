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

#ifndef GLUS_COMPLEX_H_
#define GLUS_COMPLEX_H_

/**
 * Calculates the absolute of a complex number.
 *
 * @param complex   The complex number from which the absolute is calculated.
 *
 * @result The resulting value.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusComplexAbsolutec(const GLUScomplex* complex);

/**
 * Calculates the complex conjugate.
 *
 * @param result	The resulting complex conjugate.
 * @param complex   The complex number from which the conjugate is calculated.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusComplexConjugatec(GLUScomplex* result, const GLUScomplex* complex);

/**
 * Calculates the sum of two complex numbers.
 *
 * @param result	The resulting complex number.
 * @param complex0  First complex number.
 * @param complex1  Second complex number.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusComplexAddComplexc(GLUScomplex* result, const GLUScomplex* complex0, const GLUScomplex* complex1);

/**
 * Calculates the difference of two complex numbers.
 *
 * @param result	The resulting complex number.
 * @param complex0  First complex number.
 * @param complex1  Second complex number.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusComplexSubtractComplexc(GLUScomplex* result, const GLUScomplex* complex0, const GLUScomplex* complex1);

/**
 * Multiplies two complex numbers.
 *
 * @param result	The resulting complex number.
 * @param complex0  First complex number.
 * @param complex1  Second complex number.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusComplexMultiplyComplexc(GLUScomplex* result, const GLUScomplex* complex0, const GLUScomplex* complex1);

/**
 * Divides two complex numbers.
 *
 * @param result	The resulting complex number.
 * @param complex0  First complex number.
 * @param complex1  Second complex number.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusComplexDivideComplexc(GLUScomplex* result, const GLUScomplex* complex0, const GLUScomplex* complex1);

/**
 * Multiplies a complex number with a real number.
 *
 * @param result	The resulting complex number.
 * @param complex	Complex number.
 * @param scalar  	The scalar to multiply with.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusComplexMultiplyScalarc(GLUScomplex* result, const GLUScomplex* complex, const GLUSfloat scalar);

/**
 * Calculates the exponential of complex number.
 *
 * @param result	The resulting complex number.
 * @param complex	Complex number.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusComplexExpComplexc(GLUScomplex* result, const GLUScomplex* complex);

/**
 * Calculates the root of unity.
 *
 * @param result The root of unity.
 * @param n		 The nth root of unity.
 * @param k		 k element. Maybe negative.
 * @param dir 	 The direction. Negative is clockwise.
 */
GLUSAPI GLUSvoid glusComplexRootOfUnityc(GLUScomplex* result, const GLUSint n, const GLUSint k, const GLUSfloat dir);

#endif /* GLUS_COMPLEX_H_ */
