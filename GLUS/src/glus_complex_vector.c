/*
 * GLUS - OpenGL 3 and 4 Utilities. Copyright (C) 2010 - 2013 Norbert Nopper
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

GLUSvoid GLUSAPIENTRY glusVectorNCopyc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n)
{
	GLUSint i;

	for (i = 0; i < n; i++)
	{
		result[i] = vector[i];
	}
}

GLUSvoid GLUSAPIENTRY glusVectorNConvertToFloatc(GLUSfloat* result, const GLUScomplex* vector, const GLUSint n)
{
	GLUSint i;

	for (i = 0; i < n; i++)
	{
		result[2 * i + 0] = crealf(vector[i]);
		result[2 * i + 1] = cimagf(vector[i]);
	}
}

GLUSvoid GLUSAPIENTRY glusVectorNConvertToComplexc(GLUScomplex* result, const GLUSfloat* vector, const GLUSint n)
{
	GLUSint i;

	for (i = 0; i < n; i++)
	{
		result[i] = vector[2 * i + 0] + vector[2 * i + 1] * I;
	}
}

GLUSvoid GLUSAPIENTRY glusVectorNMultiplyScalarc(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n, const GLUScomplex scalar)
{
	GLUSint i;

	for (i = 0; i < n; i++)
	{
		result[i] = vector[i] * scalar;
	}
}

GLUSvoid GLUSAPIENTRY glusVectorNConjugatec(GLUScomplex* result, const GLUScomplex* vector, const GLUSint n)
{
	GLUSint i;

	for (i = 0; i < n; i++)
	{
		result[i] = conjf(vector[i]);
	}
}
