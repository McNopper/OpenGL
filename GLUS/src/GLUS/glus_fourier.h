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

#ifndef GLUS_FOURIER_H_
#define GLUS_FOURIER_H_

/**
 * Calculates the root of unity.
 *
 * @param result The root of unity.
 * @param n		 The nth root of unity.
 * @param k		 k element. Maybe negative.
 */
GLUSAPI GLUSvoid glusRootOfUnityc(GLUScomplex* result, const GLUSint n, const GLUSint k);

/**
 * Performs a direct fourier transform on a given vector with N elements.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if transform succeeded.
 */
GLUSAPI GLUSboolean glusDirectFourierTransformc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Performs an inverse direct fourier transform on a given vector with N elements.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if transform succeeded.
 */
GLUSAPI GLUSboolean glusDirectFourierTransformInversec(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Performs a fast fourier transform on a given vector with N elements, using a recursive algorithm.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if transform succeeded.
 */
GLUSAPI GLUSboolean glusFastFourierTransformRecursivec(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Performs an inverse fast fourier transform on a given vector with N elements, using a recursive algorithm.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if transform succeeded.
 */
GLUSAPI GLUSboolean glusFastFourierTransformInverseRecursivec(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Shuffles a vector with N elements, that it can be used for a FFT butterfly algorithm.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if shuffle succeeded.
 */
GLUSAPI GLUSboolean glusFastFourierTransformButterflyShufflec(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Shuffles a index vector with N elements, that the indices can be used for a FFT butterfly algorithm.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if shuffle succeeded.
 */
GLUSAPI GLUSboolean glusFastFourierTransformButterflyShufflei(GLUSint* result, const GLUSint* vector, const GLUSint n);

/**
 * Performs a fast fourier transform on a given vector with N elements, using a butterfly algorithm.
 * Shuffling of the elements is done in this function.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if transform succeeded.
 */
GLUSAPI GLUSboolean glusFastFourierTransformButterflyc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Performs an inverse fast fourier transform on a given vector with N elements, using a butterfly algorithm.
 * Shuffling of the elements is done in this function.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if transform succeeded.
 */
GLUSAPI GLUSboolean glusFastFourierTransformInverseButterflyc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

#endif /* GLUS_FOURIER_H_ */
