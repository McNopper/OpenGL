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

GLUSfloat GLUSAPIENTRY glusComplexAbsolutec(const GLUScomplex* complex)
{
	return sqrtf(complex->real * complex->real + complex->imaginary * complex->imaginary);
}

GLUSvoid GLUSAPIENTRY glusComplexConjugatec(GLUScomplex* result, const GLUScomplex* complex)
{
	result->real = complex->real;
	result->imaginary = -complex->imaginary;
}

GLUSvoid GLUSAPIENTRY glusComplexAddComplexc(GLUScomplex* result, const GLUScomplex* complex0, const GLUScomplex* complex1)
{
	result->real = complex0->real + complex1->real;
	result->imaginary = complex0->imaginary + complex1->imaginary;
}

GLUSvoid GLUSAPIENTRY glusComplexSubtractComplexc(GLUScomplex* result, const GLUScomplex* complex0, const GLUScomplex* complex1)
{
	result->real = complex0->real - complex1->real;
	result->imaginary = complex0->imaginary - complex1->imaginary;
}

GLUSvoid GLUSAPIENTRY glusComplexMultiplyComplexc(GLUScomplex* result, const GLUScomplex* complex0, const GLUScomplex* complex1)
{
	GLUScomplex temp;

	temp.real = complex0->real * complex1->real - complex0->imaginary * complex1->imaginary;
	temp.imaginary = complex0->real * complex1->imaginary + complex1->real * complex0->imaginary;

	result->real = temp.real;
	result->imaginary = temp.imaginary;
}

GLUSvoid GLUSAPIENTRY glusComplexDivideComplexc(GLUScomplex* result, const GLUScomplex* complex0, const GLUScomplex* complex1)
{
	GLUScomplex temp;

	GLUSfloat divisor = complex1->real * complex1->real + complex1->imaginary * complex1->imaginary;

	temp.real = (complex0->real * complex1->real + complex0->imaginary * complex1->imaginary) / divisor;
	temp.imaginary = (complex1->real * complex0->imaginary - complex0->real * complex1->imaginary) / divisor;

	result->real = temp.real;
	result->imaginary = temp.imaginary;
}

GLUSvoid GLUSAPIENTRY glusComplexMultiplyScalarc(GLUScomplex* result, const GLUScomplex* complex, const GLUSfloat scalar)
{
	result->real = complex->real * scalar;
	result->imaginary = complex->imaginary * scalar;
}

GLUSvoid GLUSAPIENTRY glusComplexExpComplexc(GLUScomplex* result, const GLUScomplex* complex)
{
	GLUScomplex temp;

	temp.real = expf(complex->real) * cosf(complex->imaginary);
	temp.imaginary = expf(complex->real) * sinf(complex->imaginary);

	result->real = temp.real;
	result->imaginary = temp.imaginary;
}

GLUSvoid glusComplexRootOfUnityc(GLUScomplex* result, const GLUSint n, const GLUSint k, const GLUSfloat dir)
{
	result->real = cosf(2.0f * GLUS_PI * (GLUSfloat)k / (GLUSfloat)n);
	result->imaginary = dir * sinf(2.0f * GLUS_PI * (GLUSfloat)k / (GLUSfloat)n);
}
