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
 * Performs a direct fourier transform on a given vector with N elements.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if transform succeeded.
 */
GLUSAPI GLUSboolean glusFourierDFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Performs an inverse direct fourier transform on a given vector with N elements.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if transform succeeded.
 */
GLUSAPI GLUSboolean glusFourierInverseDFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Performs a fast fourier transform on a given vector with N elements, using a recursive algorithm.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if transform succeeded.
 */
GLUSAPI GLUSboolean glusFourierRecursiveFFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Performs an inverse fast fourier transform on a given vector with N elements, using a recursive algorithm.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if transform succeeded.
 */
GLUSAPI GLUSboolean glusFourierRecursiveInverseFFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Shuffles a vector with N elements, that it can be used for a FFT butterfly algorithm.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if shuffle succeeded.
 */
GLUSAPI GLUSboolean glusFourierButterflyShuffleFFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

/**
 * Shuffles a index vector with N elements, that the indices can be used for a FFT butterfly algorithm.
 *
 * @param result The transformed vector.
 * @param vector The source vector.
 * @param n 	 The number of elements.
 *
 * @return GLUS_TRUE, if shuffle succeeded.
 */
GLUSAPI GLUSboolean glusFourierButterflyShuffleFFTi(GLUSint* result, const GLUSint* vector, const GLUSint n);

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
GLUSAPI GLUSboolean glusFourierButterflyFFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

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
GLUSAPI GLUSboolean glusFourierButterflyInverseFFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n);

#endif /* GLUS_FOURIER_H_ */
