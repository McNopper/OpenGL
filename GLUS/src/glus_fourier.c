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

#include "GL/glus.h"

static GLUSboolean glusFourierIsPowerOfTwo(const GLUSint n)
{
	GLUSint test = n;

	if (test < 1)
	{
		return GLUS_FALSE;
	}

	while (!(test & 0x1))
	{
		test = test >> 1;
	}

	return test == 1;
}

GLUSboolean glusFourierDFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n)
{
	if (!result || !vector)
	{
		return GLUS_FALSE;
	}

	if (n > 0)
	{
		GLUSboolean status;

		GLUSint row, column;

		GLUSfloat scalar = 1.0f / (GLUSfloat)n;

		GLUScomplex* dftMatrix = (GLUScomplex*)glusMemoryMalloc(n * n * sizeof(GLUScomplex));

		if (!dftMatrix)
		{
			return GLUS_FALSE;
		}

		for (column = 0; column < n; column++)
		{
			for (row = 0; row < n; row++)
			{
				glusComplexRootOfUnityc(&dftMatrix[column * n + row], n, row * column, -1.0f);
			}
		}

		status = glusMatrixNxNMultiplyVectorNc(result, dftMatrix, vector, n);

		glusVectorNMultiplyScalarc(result, result, n, scalar);

		glusMemoryFree(dftMatrix);

		return status;
	}

	return GLUS_FALSE;
}

GLUSboolean glusFourierInverseDFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n)
{
	if (!result || !vector)
	{
		return GLUS_FALSE;
	}

	if (n > 0)
	{
		GLUSboolean status;

		GLUSint row, column;

		GLUScomplex* dftInverseMatrix = (GLUScomplex*)glusMemoryMalloc(n * n * sizeof(GLUScomplex));

		if (!dftInverseMatrix)
		{
			return GLUS_FALSE;
		}

		for (column = 0; column < n; column++)
		{
			for (row = 0; row < n; row++)
			{
				glusComplexRootOfUnityc(&dftInverseMatrix[column * n + row], n, row * column, 1.0f);
			}
		}

		status = glusMatrixNxNMultiplyVectorNc(result, dftInverseMatrix, vector, n);

		glusMemoryFree(dftInverseMatrix);

		return status;
	}

	return GLUS_FALSE;
}

static GLUSvoid glusFastFourierTransformRecursiveFunctionc(GLUScomplex* vector, const GLUSint n, GLUSint offset)
{
	if (n > 1)
	{
		GLUSint i, k;

		GLUSint m = n / 2;

		GLUScomplex temp;
		for (i = 1; i < m; i++)
		{
			for (k = 0; k < m - i; k++)
			{
				temp = vector[offset + 2 * k + i];
				vector[offset + 2 * k + i] = vector[offset + 2 * k + 1 + i];
				vector[offset + 2 * k + 1 + i] = temp;
			}
		}

		glusFastFourierTransformRecursiveFunctionc(vector, m, offset);
		glusFastFourierTransformRecursiveFunctionc(vector, m, offset + m);

		GLUScomplex currentW;
		currentW.real = 1.0f;
		currentW.imaginary = 0.0f;

		GLUScomplex w;
		glusComplexRootOfUnityc(&w, n, 1, 1.0f);

		for (i = 0; i < m; i++)
		{
			GLUScomplex multiply;
			GLUScomplex addition;
			GLUScomplex subtraction;

			glusComplexMultiplyComplexc(&multiply, &currentW, &vector[offset + i + m]);

			glusComplexAddComplexc(&addition, &vector[offset + i], &multiply);
			glusComplexSubtractComplexc(&subtraction, &vector[offset + i], &multiply);

			vector[offset + i] = addition;
			vector[offset + i + m] = subtraction;

			glusComplexMultiplyComplexc(&currentW, &currentW, &w);
		}
	}
	else
	{
		// c0 = v0, so do nothing.
	}
}

GLUSboolean glusFourierRecursiveFFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n)
{
	if (!result || !vector)
	{
		return GLUS_FALSE;
	}

	if (glusFourierIsPowerOfTwo(n))
	{
		GLUSfloat scalar = 1.0f / (GLUSfloat)n;

		glusVectorNCopyc(result, vector, n);

		glusVectorNConjugatec(result, result, n);

		glusFastFourierTransformRecursiveFunctionc(result, n, 0);

		glusVectorNConjugatec(result, result, n);
		glusVectorNMultiplyScalarc(result, result, n, scalar);

		return GLUS_TRUE;
	}

	return GLUS_FALSE;
}

GLUSboolean glusFourierRecursiveInverseFFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n)
{
	if (!result || !vector)
	{
		return GLUS_FALSE;
	}

	if (glusFourierIsPowerOfTwo(n))
	{
		glusVectorNCopyc(result, vector, n);

		glusFastFourierTransformRecursiveFunctionc(result, n, 0);

		return GLUS_TRUE;
	}

	return GLUS_FALSE;
}

static GLUSvoid glusFastFourierTransformButterflyShuffleFunctionc(GLUScomplex* vector, const GLUSint n, const GLUSint offset)
{
	if (n > 1)
	{
		GLUSint i, k;

		GLUSint m = n / 2;

		GLUScomplex temp;
		for (i = 1; i < m; i++)
		{
			for (k = 0; k < m - i; k++)
			{
				temp = vector[offset + 2 * k + i];
				vector[offset + 2 * k + i] = vector[offset + 2 * k + 1 + i];
				vector[offset + 2 * k + 1 + i] = temp;
			}
		}

		glusFastFourierTransformButterflyShuffleFunctionc(vector, m, offset);
		glusFastFourierTransformButterflyShuffleFunctionc(vector, m, offset + m);
	}
	else
	{
		// c0 = v0, so do nothing.
	}
}

GLUSboolean glusFourierButterflyShuffleFFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n)
{
	if (!result || !vector)
	{
		return GLUS_FALSE;
	}

	if (glusFourierIsPowerOfTwo(n))
	{
		glusVectorNCopyc(result, vector, n);

		glusFastFourierTransformButterflyShuffleFunctionc(result, n, 0);

		return GLUS_TRUE;
	}

	return GLUS_FALSE;
}

static GLUSvoid glusFastFourierTransformButterflyFunctionc(GLUScomplex* vector, const GLUSint n, const GLUSint offset)
{
	GLUSint currentStep;
	GLUSint currentSection;
	GLUSint currentButterfly;

	GLUSint numberSections;
	GLUSint numberButterfliesInSection;

	GLUSint m = n / 2;

	GLUSint steps = 0;
	GLUSint temp = n;
	while (!(temp & 0x1))
	{
		temp = temp >> 1;
		steps++;
	}

	numberSections = m;
	numberButterfliesInSection = 1;

	for (currentStep = 0; currentStep < steps; currentStep++)
	{
		for (currentSection = 0; currentSection < numberSections; currentSection++)
		{
			GLUScomplex currentW;
			currentW.real = 1.0f;
			currentW.imaginary = 0.0f;

			GLUScomplex w;
			glusComplexRootOfUnityc(&w, numberButterfliesInSection * 2, 1, 1.0f);

			for (currentButterfly = 0; currentButterfly < numberButterfliesInSection; currentButterfly++)
			{
				GLUSint leftIndex = currentButterfly + currentSection * numberButterfliesInSection * 2;
				GLUSint rightIndex = currentButterfly + numberButterfliesInSection + currentSection * numberButterfliesInSection * 2;

				GLUScomplex multiply;
				GLUScomplex addition;
				GLUScomplex subtraction;

				glusComplexMultiplyComplexc(&multiply, &currentW, &vector[rightIndex]);

				glusComplexAddComplexc(&addition, &vector[leftIndex], &multiply);
				glusComplexSubtractComplexc(&subtraction, &vector[leftIndex], &multiply);

				vector[leftIndex] = addition;
				vector[rightIndex] = subtraction;

				glusComplexMultiplyComplexc(&currentW, &currentW, &w);
			}
		}

		numberButterfliesInSection *= 2;
		numberSections /= 2;
	}
}

GLUSboolean glusFourierButterflyFFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n)
{
	if (!result || !vector)
	{
		return GLUS_FALSE;
	}

	if (glusFourierIsPowerOfTwo(n))
	{
		GLUSfloat scalar = 1.0f / (GLUSfloat)n;

		glusVectorNCopyc(result, vector, n);

		glusVectorNConjugatec(result, result, n);

		glusFourierButterflyShuffleFFTc(result, result, n);

		glusFastFourierTransformButterflyFunctionc(result, n, 0);

		glusVectorNConjugatec(result, result, n);
		glusVectorNMultiplyScalarc(result, result, n, scalar);

		return GLUS_TRUE;
	}

	return GLUS_FALSE;
}

GLUSboolean glusFourierButterflyInverseFFTc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n)
{
	if (!result || !vector)
	{
		return GLUS_FALSE;
	}

	if (glusFourierIsPowerOfTwo(n))
	{
		glusVectorNCopyc(result, vector, n);

		glusFourierButterflyShuffleFFTc(result, result, n);

		glusFastFourierTransformButterflyFunctionc(result, n, 0);

		return GLUS_TRUE;
	}

	return GLUS_FALSE;
}

static GLUSvoid glusFastFourierTransformButterflyShuffleFunctioni(GLUSint* vector, const GLUSint n, const GLUSint offset)
{
	if (n > 1)
	{
		GLUSint i, k;

		GLUSint m = n / 2;

		GLUSint temp;
		for (i = 1; i < m; i++)
		{
			for (k = 0; k < m - i; k++)
			{
				temp = vector[offset + 2 * k + i];
				vector[offset + 2 * k + i] = vector[offset + 2 * k + 1 + i];
				vector[offset + 2 * k + 1 + i] = temp;
			}
		}

		glusFastFourierTransformButterflyShuffleFunctioni(vector, m, offset);
		glusFastFourierTransformButterflyShuffleFunctioni(vector, m, offset + m);
	}
	else
	{
		// c0 = v0, so do nothing.
	}
}

GLUSboolean glusFourierButterflyShuffleFFTi(GLUSint* result, const GLUSint* vector, const GLUSint n)
{
	if (!result || !vector)
	{
		return GLUS_FALSE;
	}

	if (glusFourierIsPowerOfTwo(n))
	{
		GLUSint i;

		for (i = 0; i < n; i++)
		{
			result[i] = vector[i];
		}

		glusFastFourierTransformButterflyShuffleFunctioni(result, n, 0);

		return GLUS_TRUE;
	}

	return GLUS_FALSE;
}

