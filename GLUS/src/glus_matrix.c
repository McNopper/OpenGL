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

static GLUSboolean glusIsRowZero(const double matrix[16], GLUSint row)
{
    GLUSint column;

    for (column = 0; column < 4; column++)
    {
        if (matrix[column * 4 + row] != 0.0)
        {
            return GLUS_FALSE;
        }
    }

    return GLUS_TRUE;
}

static GLUSboolean glusIsColumnZero(const double matrix[16], GLUSint column)
{
    GLUSint row;

    for (row = 0; row < 4; row++)
    {
        if (matrix[column * 4 + row] != 0.0)
        {
            return GLUS_FALSE;
        }
    }

    return GLUS_TRUE;
}

static GLUSvoid glusDevideRowBy(double result[16], double matrix[16], GLUSint row, double  value)
{
    GLUSint column;
    // No inverse for robustness

    for (column = 0; column < 4; column++)
    {
        matrix[column * 4 + row] /= value;
        result[column * 4 + row] /= value;
    }
}

static GLUSvoid glusSwapRow(double result[16], double matrix[16], GLUSint rowOne, GLUSint rowTwo)
{
    GLUSint column;

    double temp;

    for (column = 0; column < 4; column++)
    {
        temp = matrix[column * 4 + rowOne];
        matrix[column * 4 + rowOne] = matrix[column * 4 + rowTwo];
        matrix[column * 4 + rowTwo] = temp;

        temp = result[column * 4 + rowOne];
        result[column * 4 + rowOne] = result[column * 4 + rowTwo];
        result[column * 4 + rowTwo] = temp;
    }
}

static GLUSvoid glusAddRow(double result[16], double matrix[16], GLUSint rowOne, GLUSint rowTwo, double factor)
{
    GLUSint column;

    for (column = 0; column < 4; column++)
    {
        matrix[column * 4 + rowOne] += matrix[column * 4 + rowTwo] * factor;
        result[column * 4 + rowOne] += result[column * 4 + rowTwo] * factor;
    }
}

//

GLUSvoid GLUSAPIENTRY glusMatrix4x4Identityf(GLUSfloat matrix[16])
{
    matrix[0] = 1.0f;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;
    matrix[3] = 0.0f;

    matrix[4] = 0.0f;
    matrix[5] = 1.0f;
    matrix[6] = 0.0f;
    matrix[7] = 0.0f;

    matrix[8] = 0.0f;
    matrix[9] = 0.0f;
    matrix[10] = 1.0f;
    matrix[11] = 0.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3Identityf(GLUSfloat matrix[9])
{
    matrix[0] = 1.0f;
    matrix[1] = 0.0f;
    matrix[2] = 0.0f;

    matrix[3] = 0.0f;
    matrix[4] = 1.0f;
    matrix[5] = 0.0f;

    matrix[6] = 0.0f;
    matrix[7] = 0.0f;
    matrix[8] = 1.0f;
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2Identityf(GLUSfloat matrix[4])
{
    matrix[0] = 1.0f;
    matrix[1] = 0.0f;

    matrix[2] = 0.0f;
    matrix[3] = 1.0f;
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4Initf(GLUSfloat matrix[16], const GLUSfloat column0[4], const GLUSfloat column1[4], const GLUSfloat column2[4], const GLUSfloat column3[4])
{
    matrix[0] = column0[0];
    matrix[1] = column0[1];
    matrix[2] = column0[2];
    matrix[3] = column0[3];

    matrix[4] = column1[0];
    matrix[5] = column1[1];
    matrix[6] = column1[2];
    matrix[7] = column1[3];

    matrix[8] = column2[0];
    matrix[9] = column2[1];
    matrix[10] = column2[2];
    matrix[11] = column2[3];

    matrix[12] = column3[0];
    matrix[13] = column3[1];
    matrix[14] = column3[2];
    matrix[15] = column3[3];
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3Initf(GLUSfloat matrix[9], const GLUSfloat column0[3], const GLUSfloat column1[3], const GLUSfloat column2[3])
{
    matrix[0] = column0[0];
    matrix[1] = column0[1];
    matrix[2] = column0[2];

    matrix[3] = column1[0];
    matrix[4] = column1[1];
    matrix[5] = column1[2];

    matrix[6] = column2[0];
    matrix[7] = column2[1];
    matrix[8] = column2[2];
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2Initf(GLUSfloat matrix[4], const GLUSfloat column0[2], const GLUSfloat column1[2])
{
    matrix[0] = column0[0];
    matrix[1] = column0[1];

    matrix[2] = column0[1];
    matrix[3] = column1[1];
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4Copyf(GLUSfloat matrix[16], const GLUSfloat source[16], const GLUSboolean rotationOnly)
{
    GLUSint i;

    if (rotationOnly)
    {
        for (i = 0; i < 12; i++)
        {
            if ((i + 1) % 4 == 0)
            {
                continue;
            }

            matrix[i] = source[i];
        }

        matrix[3] = 0.0f;
        matrix[7] = 0.0f;
        matrix[11] = 0.0f;

        matrix[12] = 0.0f;
        matrix[13] = 0.0f;
        matrix[14] = 0.0f;
        matrix[15] = 1.0f;
    }
    else
    {
        for (i = 0; i < 16; i++)
        {
            matrix[i] = source[i];
        }
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3Copyf(GLUSfloat matrix[9], const GLUSfloat source[9], const GLUSboolean rotationOnly)
{
    GLUSint i;

    if (rotationOnly)
    {
		for (i = 0; i < 6; i++)
		{
			if ((i + 1) % 3 == 0)
			{
				continue;
			}

			matrix[i] = source[i];
		}

		matrix[2] = 0.0f;
		matrix[5] = 0.0f;

		matrix[6] = 0.0f;
		matrix[7] = 0.0f;
		matrix[8] = 1.0f;
	}
    else
    {
    	for (i = 0; i < 9; i++)
    	{
    		matrix[i] = source[i];
    	}
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2Copyf(GLUSfloat matrix[4], const GLUSfloat source[4])
{
    GLUSint i;

    for (i = 0; i < 4; i++)
    {
        matrix[i] = source[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4ExtractMatrix3x3f(GLUSfloat matrix[9], const GLUSfloat source[16])
{
    matrix[0] = source[0];
    matrix[1] = source[1];
    matrix[2] = source[2];

    matrix[3] = source[4];
    matrix[4] = source[5];
    matrix[5] = source[6];

    matrix[6] = source[8];
    matrix[7] = source[9];
    matrix[8] = source[10];
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4ExtractMatrix2x2f(GLUSfloat matrix[4], const GLUSfloat source[16])
{
    matrix[0] = source[0];
    matrix[1] = source[1];

    matrix[2] = source[4];
    matrix[3] = source[5];
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3ExtractMatrix2x2f(GLUSfloat matrix[4], const GLUSfloat source[9])
{
    matrix[0] = source[0];
    matrix[1] = source[1];

    matrix[2] = source[3];
    matrix[3] = source[4];
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3CreateMatrix4x4f(GLUSfloat matrix[16], const GLUSfloat source[9])
{
	matrix[0] = source[0];
	matrix[1] = source[1];
	matrix[2] = source[2];
	matrix[3] = 0.0f;

	matrix[4] = source[3];
	matrix[5] = source[4];
	matrix[6] = source[5];
	matrix[7] = 0.0f;

	matrix[8] = source[6];
	matrix[9] = source[7];
	matrix[10] = source[8];
	matrix[11] = 0.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2CreateMatrix4x4f(GLUSfloat matrix[16], const GLUSfloat source[4])
{
	matrix[0] = source[0];
	matrix[1] = source[1];
	matrix[2] = 0.0f;
	matrix[3] = 0.0f;

	matrix[4] = source[2];
	matrix[5] = source[3];
	matrix[6] = 0.0f;
	matrix[7] = 0.0f;

	matrix[8] = 0.0f;
	matrix[9] = 0.0f;
	matrix[10] = 1.0f;
	matrix[11] = 0.0f;

	matrix[12] = 0.0f;
	matrix[13] = 0.0f;
	matrix[14] = 0.0f;
	matrix[15] = 1.0f;
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2CreateMatrix3x3f(GLUSfloat matrix[9], const GLUSfloat source[4])
{
	matrix[0] = source[0];
	matrix[1] = source[1];
	matrix[2] = 0.0f;

	matrix[3] = source[2];
	matrix[4] = source[3];
	matrix[5] = 0.0f;

	matrix[6] = 0.0f;
	matrix[7] = 0.0f;
	matrix[8] = 1.0f;
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4Addf(GLUSfloat matrix[16], const GLUSfloat matrix0[16], const GLUSfloat matrix1[16])
{
    GLUSint i;

    for (i = 0; i < 16; i++)
    {
    	matrix[i] = matrix0[i] + matrix1[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3Addf(GLUSfloat matrix[9], const GLUSfloat matrix0[9], const GLUSfloat matrix1[9])
{
    GLUSint i;

    for (i = 0; i < 9; i++)
    {
    	matrix[i] = matrix0[i] + matrix1[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2Addf(GLUSfloat matrix[4], const GLUSfloat matrix0[4], const GLUSfloat matrix1[4])
{
    GLUSint i;

    for (i = 0; i < 4; i++)
    {
    	matrix[i] = matrix0[i] + matrix1[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4Subtractf(GLUSfloat matrix[16], const GLUSfloat matrix0[16], const GLUSfloat matrix1[16])
{
    GLUSint i;

    for (i = 0; i < 16; i++)
    {
    	matrix[i] = matrix0[i] - matrix1[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3Subtractf(GLUSfloat matrix[9], const GLUSfloat matrix0[9], const GLUSfloat matrix1[9])
{
    GLUSint i;

    for (i = 0; i < 9; i++)
    {
    	matrix[i] = matrix0[i] - matrix1[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2Subtractf(GLUSfloat matrix[4], const GLUSfloat matrix0[4], const GLUSfloat matrix1[4])
{
    GLUSint i;

    for (i = 0; i < 4; i++)
    {
    	matrix[i] = matrix0[i] - matrix1[i];
    }
}

GLUSfloat GLUSAPIENTRY glusMatrix4x4Determinantf(const GLUSfloat matrix[16])
{
	GLUSint row, column, inner_row, inner_column, index;
	GLUSfloat det = 0.0f;
	GLUSfloat sign = 1.0f;
	GLUSfloat innerMatrix[9];

	column = 0;

	for (row = 0; row < 4; row++)
	{
		index = 0;

		for (inner_column = 0; inner_column < 4; inner_column++)
		{
			if (inner_column == column)
			{
				continue;
			}

			for (inner_row = 0; inner_row < 4; inner_row++)
			{
				if (inner_row == row)
				{
					continue;
				}

				innerMatrix[index] = matrix[inner_row + 4 * inner_column];

				index++;
			}
		}

		det += sign * matrix[row + 4 * column] * glusMatrix3x3Determinantf(innerMatrix);

		sign *= -1.0f;
	}

	return det;
}

GLUSfloat GLUSAPIENTRY glusMatrix3x3Determinantf(const GLUSfloat matrix[9])
{
	return matrix[0] * matrix[4] * matrix[8] + matrix[3] * matrix[7] * matrix[2] + matrix[6] * matrix[1] * matrix[5] - matrix[6] * matrix[4] * matrix[2] - matrix[3] * matrix[1] * matrix[8] - matrix[0] * matrix[7] * matrix[5];
}

GLUSfloat GLUSAPIENTRY glusMatrix2x2Determinantf(const GLUSfloat matrix[4])
{
	return matrix[0] * matrix[3]  - matrix[2] * matrix[1];
}

GLUSboolean GLUSAPIENTRY glusMatrix4x4Inversef(GLUSfloat matrix[16])
{
    GLUSint i;

    GLUSint column;
    GLUSint row;

    double matrix_as_double[16] = {1.0, 0.0, 0.0, 0.0,  0.0, 1.0, 0.0, 0.0,  0.0, 0.0, 1.0, 0.0,  0.0, 0.0, 0.0, 1.0};
    double copy[16];

    //
    // Copy the original matrix as we want to manipulate it
    //
    for (i = 0; i < 16; i++)
    {
        copy[i] = (double)matrix[i];
    }

    //
    // Make triangle form
    //
    for (column = 0; column < 4; column++)
    {
        GLUSint row;

        for (row = column; row < 4; row++)
        {
        	//
            // Is row all zero, then return false
            //
            if (glusIsRowZero(copy, row))
            {
                return GLUS_FALSE;
            }

            //
            // Divide, if not zero, by copy[column*4+row]
            //
            if (copy[column * 4 + row] != 0.0)
            {
                glusDevideRowBy(matrix_as_double, copy, row, copy[column * 4 + row]);
            }
        }

        //
        // Is column all zero, then return false
        //
        if (glusIsColumnZero(copy, column))
        {
            return GLUS_FALSE;
        }

        //
        // Is pivot [column*4+row] = 1.0
        //
        for (row = column + 1; row < 4; row++)
        {
            if (copy[column * 4 + row] == 1.0)
            {
                //
                // Swap with pivot row = column
                //
                glusSwapRow(matrix_as_double, copy, column, row);

                break;
            }
        }

        for (row = column + 1; row < 4; row++)
        {
            //
            // Subtract, [column*4+row] not zero, current row minus pivot row = column
            //
            if (copy[column * 4 + row] != 0.0)
            {
                glusAddRow(matrix_as_double, copy, row, column, -1.0);
            }
        }
    }

    //
    // Make diagonal form
    //
    for (column = 3; column >= 0; column--)
    {
        for (row = column - 1; row >= 0; row--)
        {
            //
            // Subtract, if [column*4+row] not zero, current row minus pivot row = column with factor [column*4+row]
            //
            if (copy[column * 4 + row] != 0.0)
            {
                glusAddRow(matrix_as_double, copy, row, column, -copy[column * 4 + row]);
            }
        }
    }

    for (i = 0; i < 16; i++)
    {
        matrix[i] = (float)matrix_as_double[i];
    }

    return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusMatrix3x3Inversef(GLUSfloat matrix[9])
{
    GLUSint i, j;
    GLUSint index[4];
    GLUSint counter;
    GLUSint column;
    GLUSint row;

    GLUSfloat det;
    GLUSfloat temp[9];
    GLUSfloat sign;

    det = matrix[0] * matrix[4] * matrix[8] + matrix[3] * matrix[7] * matrix[2] + matrix[6] * matrix[1] * matrix[5] - matrix[6] * matrix[4] * matrix[2] - matrix[3] * matrix[1] * matrix[8] - matrix[0] * matrix[7] * matrix[5];

    if (det == 0.0f)
    {
        return GLUS_FALSE;
    }

    glusMatrix3x3Copyf(temp, matrix, GLUS_FALSE);

    for (i = 0; i < 9; i++)
    {
        column = i / 3;
        row = i % 3;

        counter = 0;

        for (j = 0; j < 9; j++)
        {
            // Leave out column depending on given row and row depending on given column.
            if (j / 3 == row || j % 3 == column)
            {
                continue;
            }

            index[counter] = j;

            counter++;
        }

        sign = (column + row) % 2 == 0 ? 1.0f : -1.0f;

        matrix[i] = sign * (temp[index[0]] * temp[index[3]] - temp[index[2]] * temp[index[1]]) / det;
    }

    return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusMatrix2x2Inversef(GLUSfloat matrix[4])
{
    GLUSfloat det;
    GLUSfloat temp[4];

    det = matrix[0] * matrix[3]  - matrix[2] * matrix[1];

    if (det == 0.0f)
    {
        return GLUS_FALSE;
    }

    glusMatrix2x2Copyf(temp, matrix);

    matrix[0] = temp[3] / det;
    matrix[1] = -temp[1] / det;
    matrix[2] = -temp[2] / det;
    matrix[3] = temp[0] / det;

    return GLUS_TRUE;
}


GLUSvoid GLUSAPIENTRY glusMatrix4x4InverseRigidBodyf(GLUSfloat matrix[16])
{
	GLUSfloat scales[3];
    GLUSfloat inverseScale[16];
    GLUSfloat inverseRotation[16];
    GLUSfloat inverseTranslation[16];

    glusMatrix4x4GetScale(scales, matrix);
    glusMatrix4x4Identityf(inverseScale);
    // Square of it, as rotation still contains the scale
    inverseScale[0] = 1.0f / (scales[0] * scales[0]);
    inverseScale[5] = 1.0f / (scales[1] * scales[1]);
    inverseScale[10] = 1.0f / (scales[2] * scales[2]);

    glusMatrix4x4Copyf(inverseRotation, matrix, GLUS_TRUE);
    glusMatrix4x4Transposef(inverseRotation);

    glusMatrix4x4Identityf(inverseTranslation);
    inverseTranslation[12] = -matrix[12];
    inverseTranslation[13] = -matrix[13];
    inverseTranslation[14] = -matrix[14];

    glusMatrix4x4Multiplyf(matrix, inverseRotation, inverseTranslation);
    glusMatrix4x4Multiplyf(matrix, inverseScale, matrix);
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3InverseRigidBodyf(GLUSfloat matrix[9], const GLUSboolean is2D)
{
	if (is2D)
	{
		GLUSfloat scales[3];
		GLUSfloat inverseScale[9];
		GLUSfloat inverseRotation[9];
		GLUSfloat inverseTranslation[9];

		glusMatrix3x3GetScale(scales, matrix);
		glusMatrix3x3Identityf(inverseScale);
		// Square of it, as rotation still contains the scale
		inverseScale[0] = 1.0f / (scales[0] * scales[0]);
		inverseScale[4] = 1.0f / (scales[1] * scales[1]);
		// Last column is 2D translate

		glusMatrix3x3Copyf(inverseRotation, matrix, is2D);
		glusMatrix3x3Transposef(inverseRotation);

	    glusMatrix3x3Identityf(inverseTranslation);
	    inverseTranslation[6] = -matrix[6];
	    inverseTranslation[7] = -matrix[7];

	    glusMatrix3x3Multiplyf(matrix, inverseRotation, inverseTranslation);
	    glusMatrix3x3Multiplyf(matrix, inverseScale, matrix);
	}
	else
	{
		GLUSfloat scales[3];
		GLUSfloat inverseScale[9];
		GLUSfloat inverseRotation[9];

		glusMatrix3x3GetScale(scales, matrix);
		glusMatrix3x3Identityf(inverseScale);
		// Square of it, as rotation still contains the scale
		inverseScale[0] = 1.0f / (scales[0] * scales[0]);
		inverseScale[4] = 1.0f / (scales[1] * scales[1]);
		inverseScale[8] = 1.0f / (scales[2] * scales[2]);

		glusMatrix3x3Copyf(inverseRotation, matrix, is2D);
		glusMatrix3x3Transposef(inverseRotation);

		glusMatrix3x3Multiplyf(matrix, inverseScale, inverseRotation);
	}
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2InverseRigidBodyf(GLUSfloat matrix[4])
{
	GLUSfloat scales[2];
    GLUSfloat inverseScale[4];
    GLUSfloat inverseRotation[4];

    glusMatrix2x2GetScale(scales, matrix);
    glusMatrix2x2Identityf(inverseScale);
    // Square of it, as rotation still contains the scale
    inverseScale[0] = 1.0f / (scales[0] * scales[0]);
    inverseScale[3] = 1.0f / (scales[1] * scales[1]);

    glusMatrix2x2Copyf(inverseRotation, matrix);
    glusMatrix2x2Transposef(inverseRotation);

    glusMatrix4x4Multiplyf(matrix, inverseScale, inverseRotation);
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4Transposef(GLUSfloat matrix[16])
{
    GLUSint column;
    GLUSint row;

    GLUSfloat temp[16];

    glusMatrix4x4Copyf(temp, matrix, GLUS_FALSE);

    for (column = 0; column < 4; column++)
    {
        for (row = 0; row < 4; row++)
        {
            matrix[row * 4 + column] = temp[column * 4 + row];
        }
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3Transposef(GLUSfloat matrix[9])
{
    GLUSint column;
    GLUSint row;

    GLUSfloat temp[9];

    glusMatrix3x3Copyf(temp, matrix, GLUS_FALSE);

    for (column = 0; column < 3; column++)
    {
        for (row = 0; row < 3; row++)
        {
            matrix[row * 3 + column] = temp[column * 3 + row];
        }
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2Transposef(GLUSfloat matrix[4])
{
    GLUSint column;
    GLUSint row;

    GLUSfloat temp[4];

    glusMatrix2x2Copyf(temp, matrix);

    for (column = 0; column < 2; column++)
    {
        for (row = 0; row < 2; row++)
        {
            matrix[row * 2 + column] = temp[column * 2 + row];
        }
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4Translatef(GLUSfloat matrix[16], const GLUSfloat x, const GLUSfloat y, const GLUSfloat z)
{
    GLUSfloat temp[16];

    glusMatrix4x4Identityf(temp);

    temp[12] = x;
    temp[13] = y;
    temp[14] = z;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3Translatef(GLUSfloat matrix[9], const GLUSfloat x, const GLUSfloat y)
{
    GLUSfloat temp[9];

    glusMatrix3x3Identityf(temp);

    temp[6] = x;
    temp[7] = y;

    glusMatrix3x3Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4Rotatef(GLUSfloat matrix[16], const GLUSfloat angle, const GLUSfloat x, const GLUSfloat y, const GLUSfloat z)
{
    GLUSfloat temp[16];

    GLUSfloat s = sinf(2.0f * GLUS_PI * angle / 360.0f);
    GLUSfloat c = cosf(2.0f * GLUS_PI * angle / 360.0f);

    GLUSfloat vector[3] = { x, y, z };

    GLUSfloat xn, yn, zn;

    glusVector3Normalizef(vector);

    xn = vector[0];
    yn = vector[1];
    zn = vector[2];

    glusMatrix4x4Identityf(temp);

    temp[0] = xn * xn * (1 - c) + c;
    temp[1] = xn * yn * (1 - c) + zn * s;
    temp[2] = xn * zn * (1 - c) - yn * s;

    temp[4] = xn * yn * (1 - c) - zn * s;
    temp[5] = yn * yn * (1 - c) + c;
    temp[6] = yn * zn * (1 - c) + xn * s;

    temp[8] = xn * zn * (1 - c) + yn * s;
    temp[9] = yn * zn * (1 - c) - xn * s;
    temp[10] = zn * zn * (1 - c) + c;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3Rotatef(GLUSfloat matrix[9], const GLUSfloat angle, const GLUSfloat x, const GLUSfloat y, const GLUSfloat z)
{
    GLUSfloat temp[9];

    GLUSfloat s = sinf(2.0f * GLUS_PI * angle / 360.0f);
    GLUSfloat c = cosf(2.0f * GLUS_PI * angle / 360.0f);

    GLUSfloat vector[3] = { x, y, z };

    GLUSfloat xn, yn, zn;

    glusVector3Normalizef(vector);

    xn = vector[0];
    yn = vector[1];
    zn = vector[2];

    glusMatrix3x3Identityf(temp);

    temp[0] = xn * xn * (1 - c) + c;
    temp[1] = xn * yn * (1 - c) + zn * s;
    temp[2] = xn * zn * (1 - c) - yn * s;

    temp[3] = xn * yn * (1 - c) - zn * s;
    temp[4] = yn * yn * (1 - c) + c;
    temp[5] = yn * zn * (1 - c) + xn * s;

    temp[6] = xn * zn * (1 - c) + yn * s;
    temp[7] = yn * zn * (1 - c) - xn * s;
    temp[8] = zn * zn * (1 - c) + c;

    glusMatrix3x3Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2Rotatef(GLUSfloat matrix[4], const GLUSfloat angle)
{
    GLUSfloat temp[4];

    GLUSfloat s = sinf(2.0f * GLUS_PI * angle / 360.0f);
    GLUSfloat c = cosf(2.0f * GLUS_PI * angle / 360.0f);

    glusMatrix2x2Identityf(temp);

    temp[0] = c;
    temp[1] = s;

    temp[2] = -s;
    temp[3] = c;

    glusMatrix2x2Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRxf(GLUSfloat matrix[16], const GLUSfloat angle)
{
    GLUSfloat temp[16];

    GLUSfloat s = sinf(2.0f * GLUS_PI * angle / 360.0f);
    GLUSfloat c = cosf(2.0f * GLUS_PI * angle / 360.0f);

    glusMatrix4x4Identityf(temp);

    temp[5] = c;
    temp[6] = s;

    temp[9] = -s;
    temp[10] = c;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRxf(GLUSfloat matrix[9], const GLUSfloat angle)
{
    GLUSfloat temp[9];

    GLUSfloat s = sinf(2.0f * GLUS_PI * angle / 360.0f);
    GLUSfloat c = cosf(2.0f * GLUS_PI * angle / 360.0f);

    glusMatrix3x3Identityf(temp);

    temp[4] = c;
    temp[5] = s;

    temp[7] = -s;
    temp[8] = c;

    glusMatrix3x3Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRyf(GLUSfloat matrix[16], const GLUSfloat angle)
{
    GLUSfloat temp[16];

    GLUSfloat s = sinf(2.0f * GLUS_PI * angle / 360.0f);
    GLUSfloat c = cosf(2.0f * GLUS_PI * angle / 360.0f);

    glusMatrix4x4Identityf(temp);

    temp[0] = c;
    temp[2] = -s;

    temp[8] = s;
    temp[10] = c;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRyf(GLUSfloat matrix[9], const GLUSfloat angle)
{
    GLUSfloat temp[9];

    GLUSfloat s = sinf(2.0f * GLUS_PI * angle / 360.0f);
    GLUSfloat c = cosf(2.0f * GLUS_PI * angle / 360.0f);

    glusMatrix3x3Identityf(temp);

    temp[0] = c;
    temp[2] = -s;

    temp[6] = s;
    temp[8] = c;

    glusMatrix3x3Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRzf(GLUSfloat matrix[16], const GLUSfloat angle)
{
    GLUSfloat temp[16];

    GLUSfloat s = sinf(2.0f * GLUS_PI * angle / 360.0f);
    GLUSfloat c = cosf(2.0f * GLUS_PI * angle / 360.0f);

    glusMatrix4x4Identityf(temp);

    temp[0] = c;
    temp[1] = s;

    temp[4] = -s;
    temp[5] = c;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRzf(GLUSfloat matrix[9], const GLUSfloat angle)
{
    GLUSfloat temp[9];

    GLUSfloat s = sinf(2.0f * GLUS_PI * angle / 360.0f);
    GLUSfloat c = cosf(2.0f * GLUS_PI * angle / 360.0f);

    glusMatrix3x3Identityf(temp);

    temp[0] = c;
    temp[1] = s;

    temp[3] = -s;
    temp[4] = c;

    glusMatrix3x3Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRzRxRyf(GLUSfloat matrix[16], const GLUSfloat anglez, const GLUSfloat anglex, const GLUSfloat angley)
{
    GLUSfloat temp[16];

    GLUSfloat rz = 2.0f * GLUS_PI * anglez / 360.0f;
    GLUSfloat rx = 2.0f * GLUS_PI * anglex / 360.0f;
    GLUSfloat ry = 2.0f * GLUS_PI * angley / 360.0f;
    GLUSfloat sy = sinf(ry);
    GLUSfloat cy = cosf(ry);
    GLUSfloat sx = sinf(rx);
    GLUSfloat cx = cosf(rx);
    GLUSfloat sz = sinf(rz);
    GLUSfloat cz = cosf(rz);

    glusMatrix4x4Identityf(temp);

    temp[0] = cy * cz - sx * sy * sz;
    temp[1] = cz * sx * sy + cy * sz;
    temp[2] = -cx * sy;

    temp[4] = -cx * sz;
    temp[5] = cx * cz;
    temp[6] = sx;

    temp[8] = cz * sy + cy * sx * sz;
    temp[9] = -cy * cz * sx + sy * sz;
    temp[10] = cx * cy;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRzRyRxf(GLUSfloat matrix[16], const GLUSfloat anglez, const GLUSfloat angley, const GLUSfloat anglex)
{
    GLUSfloat temp[16];

    GLUSfloat rz = 2.0f * GLUS_PI * anglez / 360.0f;
    GLUSfloat ry = 2.0f * GLUS_PI * angley / 360.0f;
    GLUSfloat rx = 2.0f * GLUS_PI * anglex / 360.0f;
    GLUSfloat sx = sinf(rx);
    GLUSfloat cx = cosf(rx);
    GLUSfloat sy = sinf(ry);
    GLUSfloat cy = cosf(ry);
    GLUSfloat sz = sinf(rz);
    GLUSfloat cz = cosf(rz);

    glusMatrix4x4Identityf(temp);

    temp[0] = cy * cz;
    temp[1] = cy * sz;
    temp[2] = -sy;

    temp[4] = -cx * sz + cz * sx * sy;
    temp[5] = cx * cz + sx * sy * sz;
    temp[6] = cy * sx;

    temp[8] = sz * sx + cx * cz * sy;
    temp[9] = -cz * sx + cx * sy * sz;
    temp[10] = cx * cy;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRzRxRyf(GLUSfloat matrix[9], const GLUSfloat anglez, const GLUSfloat anglex, const GLUSfloat angley)
{
    GLUSfloat temp[9];

    GLUSfloat rz = 2.0f * GLUS_PI * anglez / 360.0f;
    GLUSfloat rx = 2.0f * GLUS_PI * anglex / 360.0f;
    GLUSfloat ry = 2.0f * GLUS_PI * angley / 360.0f;
    GLUSfloat sy = sinf(ry);
    GLUSfloat cy = cosf(ry);
    GLUSfloat sx = sinf(rx);
    GLUSfloat cx = cosf(rx);
    GLUSfloat sz = sinf(rz);
    GLUSfloat cz = cosf(rz);

    glusMatrix3x3Identityf(temp);

    temp[0] = cy * cz - sx * sy * sz;
    temp[1] = cz * sx * sy + cy * sz;
    temp[2] = -cx * sy;

    temp[3] = -cx * sz;
    temp[4] = cx * cz;
    temp[5] = sx;

    temp[6] = cz * sy + cy * sx * sz;
    temp[7] = -cy * cz * sx + sy * sz;
    temp[8] = cx * cy;

    glusMatrix3x3Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRzRyRxf(GLUSfloat matrix[9], const GLUSfloat anglez, const GLUSfloat angley, const GLUSfloat anglex)
{
    GLUSfloat temp[9];

    GLUSfloat rz = 2.0f * GLUS_PI * anglez / 360.0f;
    GLUSfloat ry = 2.0f * GLUS_PI * angley / 360.0f;
    GLUSfloat rx = 2.0f * GLUS_PI * anglex / 360.0f;
    GLUSfloat sx = sinf(rx);
    GLUSfloat cx = cosf(rx);
    GLUSfloat sy = sinf(ry);
    GLUSfloat cy = cosf(ry);
    GLUSfloat sz = sinf(rz);
    GLUSfloat cz = cosf(rz);

    glusMatrix3x3Identityf(temp);

    temp[0] = cy * cz;
    temp[1] = cy * sz;
    temp[2] = -sy;

    temp[3] = -cx * sz + cz * sx * sy;
    temp[4] = cx * cz + sx * sy * sz;
    temp[5] = cy * sx;

    temp[6] = sz * sx + cx * cz * sy;
    temp[7] = -cz * sx + cx * sy * sz;
    temp[8] = cx * cy;

    glusMatrix3x3Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4Scalef(GLUSfloat matrix[16], const GLUSfloat x, const GLUSfloat y, const GLUSfloat z)
{
    GLUSfloat temp[16];

    glusMatrix4x4Identityf(temp);

    temp[0] = x;
    temp[5] = y;
    temp[10] = z;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3Scalef(GLUSfloat matrix[9], const GLUSfloat x, const GLUSfloat y, const GLUSfloat z)
{
    GLUSfloat temp[9];

    glusMatrix3x3Identityf(temp);

    temp[0] = x;
    temp[4] = y;
    temp[8] = z;

    glusMatrix3x3Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2Scalef(GLUSfloat matrix[4], const GLUSfloat x, const GLUSfloat y)
{
    GLUSfloat temp[4];

    glusMatrix2x2Identityf(temp);

    temp[0] = x;
    temp[3] = y;

    glusMatrix2x2Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4Shearf(GLUSfloat matrix[16], const GLUSfloat shxy, const GLUSfloat shxz, const GLUSfloat shyx, const GLUSfloat shyz, const GLUSfloat shzx, const GLUSfloat shzy)
{
    GLUSfloat temp[16];

    glusMatrix4x4Identityf(temp);

    temp[4] = shxy;
    temp[8] = shxz;

    temp[1] = shyx;
    temp[9] = shyz;

    temp[2] = shzx;
    temp[6] = shzy;

    glusMatrix4x4Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3Shearf(GLUSfloat matrix[9], const GLUSfloat shxy, const GLUSfloat shxz, const GLUSfloat shyx, const GLUSfloat shyz, const GLUSfloat shzx, const GLUSfloat shzy)
{
    GLUSfloat temp[9];

    glusMatrix3x3Identityf(temp);

    temp[3] = shxy;
    temp[6] = shxz;

    temp[1] = shyx;
    temp[7] = shyz;

    temp[2] = shzx;
    temp[5] = shzy;

    glusMatrix3x3Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2Shearf(GLUSfloat matrix[4], const GLUSfloat shx, const GLUSfloat shy)
{
    GLUSfloat temp[4];

    glusMatrix2x2Identityf(temp);

    temp[2] = shx;

    temp[1] = shy;

    glusMatrix2x2Multiplyf(matrix, matrix, temp);
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyVector3f(GLUSfloat result[3], const GLUSfloat matrix[16], const GLUSfloat vector[3])
{
    GLUSint i;

    GLUSfloat temp[3];

    for (i = 0; i < 3; i++)
    {
        temp[i] = matrix[i] * vector[0] + matrix[4 + i] * vector[1] + matrix[8 + i] * vector[2];
    }

    for (i = 0; i < 3; i++)
    {
        result[i] = temp[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyVector2f(GLUSfloat result[2], const GLUSfloat matrix[16], const GLUSfloat vector[2])
{
    GLUSint i;

    GLUSfloat temp[2];

    for (i = 0; i < 2; i++)
    {
        temp[i] = matrix[i] * vector[0] + matrix[4 + i] * vector[1];
    }

    for (i = 0; i < 2; i++)
    {
        result[i] = temp[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3MultiplyVector3f(GLUSfloat result[3], const GLUSfloat matrix[9], const GLUSfloat vector[3])
{
    GLUSint i;

    GLUSfloat temp[3];

    for (i = 0; i < 3; i++)
    {
        temp[i] = matrix[i] * vector[0] + matrix[3 + i] * vector[1] + matrix[6 + i] * vector[2];
    }

    for (i = 0; i < 3; i++)
    {
        result[i] = temp[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3MultiplyVector2f(GLUSfloat result[2], const GLUSfloat matrix[9], const GLUSfloat vector[2])
{
    GLUSint i;

    GLUSfloat temp[2];

    for (i = 0; i < 2; i++)
    {
        temp[i] = matrix[i] * vector[0] + matrix[3 + i] * vector[1];
    }

    for (i = 0; i < 2; i++)
    {
        result[i] = temp[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2MultiplyVector2f(GLUSfloat result[2], const GLUSfloat matrix[4], const GLUSfloat vector[2])
{
    GLUSint i;

    GLUSfloat temp[2];

    for (i = 0; i < 2; i++)
    {
        temp[i] = matrix[i] * vector[0] + matrix[2 + i] * vector[1];
    }

    for (i = 0; i < 2; i++)
    {
        result[i] = temp[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyPoint4f(GLUSfloat result[4], const GLUSfloat matrix[16], const GLUSfloat point[4])
{
    GLUSint i;

    GLUSfloat temp[4];

    for (i = 0; i < 4; i++)
    {
        temp[i] = matrix[i] * point[0] + matrix[4 + i] * point[1] + matrix[8 + i] * point[2] + matrix[12 + i] * point[3];
    }

    for (i = 0; i < 4; i++)
    {
        result[i] = temp[i];
    }

    if (result[3] != 0.0f && result[3] != 1.0f)
    {
        for (i = 0; i < 4; i++)
        {
            result[i] /= temp[3];
        }
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3MultiplyPoint3f(GLUSfloat result[3], const GLUSfloat matrix[9], const GLUSfloat point[3])
{
    GLUSint i;

    GLUSfloat temp[3];

    for (i = 0; i < 3; i++)
    {
        temp[i] = matrix[i] * point[0] + matrix[3 + i] * point[1] + matrix[6 + i] * point[2];
    }

    for (i = 0; i < 3; i++)
    {
        result[i] = temp[i];
    }

    if (result[2] != 0.0f && result[2] != 1.0f)
    {
        for (i = 0; i < 3; i++)
        {
            result[i] /= temp[2];
        }
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyPlanef(GLUSfloat result[4], const GLUSfloat matrix[16], const GLUSfloat plane[4])
{
    GLUSint i;

    GLUSfloat temp[4];

    for (i = 0; i < 4; i++)
    {
        temp[i] = matrix[i] * plane[0] + matrix[4 + i] * plane[1] + matrix[8 + i] * plane[2] + matrix[12 + i] * plane[3];
    }

    for (i = 0; i < 4; i++)
    {
        result[i] = temp[i];
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4GetEulerRzRxRyf(GLUSfloat angles[3], const GLUSfloat matrix[16])
{
	GLUSfloat scales[3];

	glusMatrix4x4GetScale(scales, matrix);

    if (matrix[5] != 0.0f)
    {
        angles[0] = glusRadToDegf(asinf(matrix[6] / scales[1]));
        angles[1] = glusRadToDegf(atan2f(-matrix[2] / scales[0], matrix[10] / scales[2]));
        angles[2] = glusRadToDegf(atan2f(-matrix[4] / scales[1], matrix[5] / scales[1]));
    }
    else
    {
        angles[0] = glusRadToDegf(asinf(matrix[6] / scales[1]));
        angles[1] = 0.0f;
        angles[2] = glusRadToDegf(atan2f(matrix[1] / scales[0], matrix[0] / scales[0]));
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4GetEulerRzRyRxf(GLUSfloat angles[3], const GLUSfloat matrix[16])
{
	GLUSfloat scales[3];

	glusMatrix4x4GetScale(scales, matrix);

    if (matrix[0] != 0.0f)
    {
        angles[0] = glusRadToDegf(atan2f(matrix[6] / scales[1], matrix[10] / scales[2]));
    	angles[1] = glusRadToDegf(asinf(-matrix[2] / scales[0]));
    	angles[2] = glusRadToDegf(atan2f(matrix[1] / scales[0], matrix[0] / scales[0]));
    }
    else
    {
        angles[0] = glusRadToDegf(atan2f(matrix[4] / scales[1], matrix[5] / scales[1]));
    	angles[1] = glusRadToDegf(asinf(-matrix[2]) / scales[0]);
    	angles[2] = 0.0f;
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3GetEulerRzRxRyf(GLUSfloat angles[3], const GLUSfloat matrix[9])
{
	GLUSfloat scales[3];

	glusMatrix3x3GetScale(scales, matrix);

    if (matrix[4] != 0.0f)
    {
        angles[0] = glusRadToDegf(asinf(matrix[5] / scales[1]));
        angles[1] = glusRadToDegf(atan2f(-matrix[2] / scales[0], matrix[8] / scales[2]));
        angles[2] = glusRadToDegf(atan2f(-matrix[3] / scales[1], matrix[4] / scales[1]));
    }
    else
    {
        angles[0] = glusRadToDegf(asinf(matrix[5] / scales[1]));
        angles[1] = 0.0f;
        angles[2] = glusRadToDegf(atan2f(matrix[1] / scales[0], matrix[0] / scales[0]));
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3GetEulerRzRyRxf(GLUSfloat angles[3], const GLUSfloat matrix[9])
{
	GLUSfloat scales[3];

	glusMatrix3x3GetScale(scales, matrix);

    if (matrix[0] != 0.0f)
    {
        angles[0] = glusRadToDegf(atan2f(matrix[5] / scales[1], matrix[8] / scales[2]));
    	angles[1] = glusRadToDegf(asinf(-matrix[2] / scales[0]));
    	angles[2] = glusRadToDegf(atan2f(matrix[1] / scales[0], matrix[0] / scales[0]));
    }
    else
    {
        angles[0] = glusRadToDegf(atan2f(matrix[3] / scales[1], matrix[4] / scales[1]));
    	angles[1] = glusRadToDegf(asinf(-matrix[2] / scales[0]));
    	angles[2] = 0.0f;
    }
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3GetAnglef(GLUSfloat* angle, const GLUSfloat matrix[9])
{
	GLUSfloat scales[3];

	glusMatrix3x3GetScale(scales, matrix);

	*angle = glusRadToDegf(acosf(matrix[0] / scales[0]));
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2GetAnglef(GLUSfloat* angle, const GLUSfloat matrix[4])
{
	GLUSfloat scales[2];

	glusMatrix2x2GetScale(scales, matrix);

	*angle = glusRadToDegf(acosf(matrix[0] / scales[0]));
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4GetScale(GLUSfloat scales[3], const GLUSfloat matrix[16])
{
	GLUSint i;

	for (i = 0; i < 3; i++)
	{
		scales[i] = sqrtf(matrix[4*i] * matrix[4*i] + matrix[4*i+1] * matrix[4*i+1] + matrix[4*i+2] * matrix[4*i+2]);
	}
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3GetScale(GLUSfloat scales[3], const GLUSfloat matrix[9])
{
	GLUSint i;

	for (i = 0; i < 3; i++)
	{
		scales[i] = sqrtf(matrix[3*i] * matrix[3*i] + matrix[3*i+1] * matrix[3*i+1] + matrix[3*i+2] * matrix[3*i+2]);
	}
}

GLUSvoid GLUSAPIENTRY glusMatrix2x2GetScale(GLUSfloat scales[2], const GLUSfloat matrix[4])
{
	GLUSint i;

	for (i = 0; i < 2; i++)
	{
		scales[i] = sqrtf(matrix[2*i] * matrix[2*i] + matrix[2*i+1] * matrix[2*i+1]);
	}
}

GLUSvoid GLUSAPIENTRY glusMatrix4x4GetTranslate(GLUSfloat translates[3], const GLUSfloat matrix[16])
{
	translates[0] = matrix[12];
	translates[1] = matrix[13];
	translates[2] = matrix[14];
}

GLUSvoid GLUSAPIENTRY glusMatrix3x3GetTranslate(GLUSfloat translates[2], const GLUSfloat matrix[9])
{
	translates[0] = matrix[6];
	translates[1] = matrix[7];
}
