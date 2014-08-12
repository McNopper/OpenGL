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

GLUSboolean GLUSAPIENTRY glusMatrixNxNMultiplyVectorNc(GLUScomplex* result, const GLUScomplex* matrix, const GLUScomplex* vector, const GLUSint n)
{
	GLUSint row, column;

	GLUScomplex* temp = (GLUScomplex*)glusMalloc(n * sizeof(GLUScomplex));

	GLUScomplex muliplication;

	if (!temp)
	{
		return GLUS_FALSE;
	}

	for (row = 0; row < n; row++)
	{
		for (column = 0; column < n; column++)
		{
			if (column == 0)
			{
				temp[row].real = 0.0f;
				temp[row].imaginary = 0.0f;
			}

			glusComplexMultiplyComplexc(&muliplication, &matrix[column * n + row], &vector[column]);
			glusComplexAddComplexc(&temp[row], &temp[row], &muliplication);
		}
	}

	for (row = 0; row < n; row++)
	{
		result[row] = temp[row];
	}

	glusFree(temp);

	return GLUS_TRUE;
}
