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

#ifndef GLUS_MATRIX_H_
#define GLUS_MATRIX_H_

/**
 * Sets the given 4x4 matrix to an identity matrix.
 *
 * @param matrix The matrix, which is set to the identity matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Identityf(GLUSfloat matrix[16]);

/**
 * Sets the given 3x3 matrix to an identity matrix.
 *
 * @param matrix The matrix, which is set to the identity matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Identityf(GLUSfloat matrix[9]);

/**
 * Sets the given 2x2 matrix to an identity matrix.
 *
 * @param matrix The matrix, which is set to the identity matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Identityf(GLUSfloat matrix[4]);

/**
 * Initializes the given 4x4 matrix with the provided column vectors.
 *
 * @param matrix The matrix, which is initalized with the given column vectors.
 * @param column0 The first column of the matrix.
 * @param column1 The second column of the matrix.
 * @param column2 The third column of the matrix.
 * @param column3 The fourth column of the matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Initf(GLUSfloat matrix[16], const GLUSfloat column0[4], const GLUSfloat column1[4], const GLUSfloat column2[4], const GLUSfloat column3[4]);

/**
 * Initializes the given 3x3 matrix with the provided column vectors.
 *
 * @param matrix The matrix, which is initalized with the given column vectors.
 * @param column0 The first column of the matrix.
 * @param column1 The second column of the matrix.
 * @param column2 The third column of the matrix.
 */

GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Initf(GLUSfloat matrix[9], const GLUSfloat column0[3], const GLUSfloat column1[3], const GLUSfloat column2[3]);

/**
 * Initializes the given 2x2 matrix with the provided column vectors.
 *
 * @param matrix The matrix, which is initalized with the given column vectors.
 * @param column0 The first column of the matrix.
 * @param column1 The second column of the matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Initf(GLUSfloat matrix[4], const GLUSfloat column0[2], const GLUSfloat column1[2]);

/**
 * Copies a 4x4 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 * @param rotationOnly If GLUS_TRUE, only the 3x3 Matrix is copied into an identity 4x4 matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Copyf(GLUSfloat matrix[16], const GLUSfloat source[16], const GLUSboolean rotationOnly);

/**
 * Copies a 3x3 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 * @param rotationOnly If GLUS_TRUE, only the 2x2 Matrix is copied into an identity 3x3 matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Copyf(GLUSfloat matrix[9], const GLUSfloat source[9], const GLUSboolean rotationOnly);

/**
 * Copies a 3x3 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Copyf(GLUSfloat matrix[4], const GLUSfloat source[4]);

/**
 * Extracts a 3x3 matrix out of a 4x4 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4ExtractMatrix3x3f(GLUSfloat matrix[9], const GLUSfloat source[16]);

/**
 * Extracts a 2x2 matrix out of a 4x4 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4ExtractMatrix2x2f(GLUSfloat matrix[4], const GLUSfloat source[16]);

/**
 * Extracts a 2x2 matrix out of a 3x3 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3ExtractMatrix2x2f(GLUSfloat matrix[4], const GLUSfloat source[9]);

/**
 * Creates a 4x4 matrix out of a 3x3 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3CreateMatrix4x4f(GLUSfloat matrix[16], const GLUSfloat source[9]);

/**
 * Creates a 4x4 matrix out of a 2x2 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2CreateMatrix4x4f(GLUSfloat matrix[16], const GLUSfloat source[4]);

/**
 * Creates a 3x3 matrix out of a 2x2 matrix.
 *
 * @param matrix The destination matrix.
 * @param source The source matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2CreateMatrix3x3f(GLUSfloat matrix[9], const GLUSfloat source[4]);

/**
 * Multiplies two 4x4 matrices: matrix0 * matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSINLINE GLUSvoid GLUSAPIENTRY glusMatrix4x4Multiplyf(GLUSfloat matrix[16], const GLUSfloat matrix0[16], const GLUSfloat matrix1[16])
{
    GLUSint i;

    GLUSfloat temp[16];

    GLUSint row;
    GLUSint column;
	for (column = 0; column < 4; column++)
	{
		for (row = 0; row < 4; row++)
		{
			temp[column * 4 + row] = 0.0f;

			for (i = 0; i < 4; i++)
			{
				temp[column * 4 + row] += matrix0[i * 4 + row] * matrix1[column * 4 + i];
			}
		}
	}

    for (i = 0; i < 16; i++)
    {
        matrix[i] = temp[i];
    }
}

/**
 * Multiplies two 3x3 matrices: matrix0 * matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSINLINE GLUSvoid GLUSAPIENTRY glusMatrix3x3Multiplyf(GLUSfloat matrix[9], const GLUSfloat matrix0[9], const GLUSfloat matrix1[9])
{
    GLUSint i;

    GLUSfloat temp[16];

    GLUSint row;
    GLUSint column;
	for (column = 0; column < 3; column++)
	{
		for (row = 0; row < 3; row++)
		{
			temp[column * 3 + row] = 0.0f;

			for (i = 0; i < 3; i++)
			{
				temp[column * 3 + row] += matrix0[i * 3 + row] * matrix1[column * 3 + i];
			}
		}
	}

    for (i = 0; i < 9; i++)
    {
        matrix[i] = temp[i];
    }
}

/**
 * Multiplies two 2x2 matrices: matrix0 * matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSINLINE GLUSvoid GLUSAPIENTRY glusMatrix2x2Multiplyf(GLUSfloat matrix[4], const GLUSfloat matrix0[4], const GLUSfloat matrix1[4])
{
    GLUSint i;

    GLUSfloat temp[16];

    GLUSint row;
    GLUSint column;
	for (column = 0; column < 2; column++)
	{
		for (row = 0; row < 2; row++)
		{
			temp[column * 2 + row] = 0.0f;

			for (i = 0; i < 2; i++)
			{
				temp[column * 2 + row] += matrix0[i * 2 + row] * matrix1[column * 2 + i];
			}
		}
	}

    for (i = 0; i < 4; i++)
    {
        matrix[i] = temp[i];
    }
}

/**
 * Adds two 4x4 matrices: matrix0 + matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Addf(GLUSfloat matrix[16], const GLUSfloat matrix0[16], const GLUSfloat matrix1[16]);

/**
 * Adds two 3x3 matrices: matrix0 + matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Addf(GLUSfloat matrix[9], const GLUSfloat matrix0[9], const GLUSfloat matrix1[9]);

/**
 * Adds two 2x2 matrices: matrix0 + matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Addf(GLUSfloat matrix[4], const GLUSfloat matrix0[4], const GLUSfloat matrix1[4]);

/**
 * Subtracts two 4x4 matrices: matrix0 - matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Subtractf(GLUSfloat matrix[16], const GLUSfloat matrix0[16], const GLUSfloat matrix1[16]);

/**
 * Subtracts two 3x3 matrices: matrix0 - matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Subtractf(GLUSfloat matrix[9], const GLUSfloat matrix0[9], const GLUSfloat matrix1[9]);

/**
 * Subtracts two 2x2 matrices: matrix0 - matrix1.
 *
 * @param matrix The resulting matrix.
 * @param matrix0 The first matrix.
 * @param matrix1 The second matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Subtractf(GLUSfloat matrix[4], const GLUSfloat matrix0[4], const GLUSfloat matrix1[4]);

/**
 * Calculates the determinant of a 4x4 matrix.
 *
 * @param matrix The used matrix.
 *
 * @return The calculated determinant.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMatrix4x4Determinantf(const GLUSfloat matrix[16]);

/**
 * Calculates the determinant of a 3x3 matrix.
 *
 * @param matrix The used matrix.
 *
 * @return The calculated determinant.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMatrix3x3Determinantf(const GLUSfloat matrix[9]);

/**
 * Calculates the determinant of a 2x2 matrix.
 *
 * @param matrix The used matrix.
 *
 * @return The calculated determinant.
 */
GLUSAPI GLUSfloat GLUSAPIENTRY glusMatrix2x2Determinantf(const GLUSfloat matrix[4]);

/**
 * Calculates the inverse of a 4x4 matrix using Gaussian Elimination.
 *
 * @param matrix The matrix to be inverted.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusMatrix4x4Inversef(GLUSfloat matrix[16]);

/**
 * Calculates the inverse of a 3x3 matrix using the determinant and adjunct of a matrix.
 *
 * @param matrix The matrix to be inverted.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusMatrix3x3Inversef(GLUSfloat matrix[9]);

/**
 * Calculates the inverse of a 2x2 matrix using the determinant and adjunct of a matrix.
 *
 * @param matrix The matrix to be inverted.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusMatrix2x2Inversef(GLUSfloat matrix[4]);

/**
 * Calculates the inverse of a 4x4 matrix by assuming it is a rigid body matrix.
 *
 * @param matrix The matrix to be inverted.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4InverseRigidBodyf(GLUSfloat matrix[16]);

/**
 * Calculates the inverse of a 3x3 matrix by assuming it is a rigid body matrix.
 *
 * @param matrix The matrix to be inverted.
 * @param is2D Set to GLUS_TRUE, if this matrix should be treated as a 2D matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3InverseRigidBodyf(GLUSfloat matrix[9], const GLUSboolean is2D);

/**
 * Calculates the inverse of a 2x2 matrix by assuming it is a rigid body matrix.
 *
 * @param matrix The matrix to be inverted.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2InverseRigidBodyf(GLUSfloat matrix[4]);

/**
 * Transposes a 4x4 matrix.
 *
 * @param matrix The matrix to be transposed.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Transposef(GLUSfloat matrix[16]);

/**
 * Transposes a 3x3 matrix.
 *
 * @param matrix The matrix to be transposed.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Transposef(GLUSfloat matrix[9]);

/**
 * Transposes a 2x2 matrix.
 *
 * @param matrix The matrix to be transposed.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Transposef(GLUSfloat matrix[4]);

/**
 * Creates a 4x4 translation matrix and multiplies it with the provided one: matrix = matrix * translateMatrix
 *
 * @param matrix The final matrix.
 * @param x Amount to translate in x direction.
 * @param y Amount to translate in y direction.
 * @param z Amount to translate in z direction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Translatef(GLUSfloat matrix[16], const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a 3x3 translation matrix and multiplies it with the provided one: matrix = matrix * translateMatrix
 *
 * @param matrix The final matrix.
 * @param x Amount to translate in x direction.
 * @param y Amount to translate in y direction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Translatef(GLUSfloat matrix[9], const GLUSfloat x, const GLUSfloat y);

/**
 * Creates a 4x4 matrix for rotation around a given axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 * @param x X coordinate of axis.
 * @param y Y coordinate of axis.
 * @param z Z coordinate of axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Rotatef(GLUSfloat matrix[16], const GLUSfloat angle, const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a 3x3 matrix for rotation around a given axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 * @param x X coordinate of axis.
 * @param y Y coordinate of axis.
 * @param z Z coordinate of axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Rotatef(GLUSfloat matrix[9], const GLUSfloat angle, const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a 2x2 matrix for a rotation around in 2D and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Rotatef(GLUSfloat matrix[4], const GLUSfloat angle);

/**
 * Creates a 4x4 matrix for rotation around the x axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRxf(GLUSfloat matrix[16], const GLUSfloat angle);

/**
 * Creates a 3x3 matrix for rotation around the x axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRxf(GLUSfloat matrix[9], const GLUSfloat angle);

/**
 * Creates a 4x4 matrix for rotation around the y axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRyf(GLUSfloat matrix[16], const GLUSfloat angle);

/**
 * Creates a 3x3 matrix for rotation around the y axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRyf(GLUSfloat matrix[9], const GLUSfloat angle);

/**
 * Creates a 4x4 matrix for rotation around the z axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRzf(GLUSfloat matrix[16], const GLUSfloat angle);

/**
 * Creates a 3x3 matrix for rotation around the z axis and multiplies it with the given one: matrix = matrix * rotateMatrix
 *
 * @param matrix The final matrix.
 * @param angle The rotation angle in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRzf(GLUSfloat matrix[9], const GLUSfloat angle);

/**
 * Creates a 4x4 matrix for rotation using Euler angles and multiplies it with the given one: matrix = matrix * Rz * Rx * Ry
 *
 * @param matrix The final matrix.
 * @param anglez The rotation angle for the z axis in degree.
 * @param anglex The rotation angle for the x axis in degree.
 * @param angley The rotation angle for the y axis in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRzRxRyf(GLUSfloat matrix[16], const GLUSfloat anglez, const GLUSfloat anglex, const GLUSfloat angley);

/**
 * Creates a 4x4 matrix for rotation using Euler angles and multiplies it with the given one: matrix = matrix * Rz * Ry * Rx
 *
 * @param matrix The final matrix.
 * @param anglez The rotation angle for the z axis in degree.
 * @param angley The rotation angle for the y axis in degree.
 * @param anglex The rotation angle for the x axis in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4RotateRzRyRxf(GLUSfloat matrix[16], const GLUSfloat anglez, const GLUSfloat angley, const GLUSfloat anglex);

/**
 * Creates a 3x3 matrix for rotation using Euler angles and multiplies it with the given one: matrix = matrix * Rz * Rx * Ry
 *
 * @param matrix The final matrix.
 * @param anglez The rotation angle for the z axis in degree.
 * @param anglex The rotation angle for the x axis in degree.
 * @param angley The rotation angle for the y axis in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRzRxRyf(GLUSfloat matrix[9], const GLUSfloat anglez, const GLUSfloat anglex, const GLUSfloat angley);

/**
 * Creates a 3x3 matrix for rotation using Euler angles and multiplies it with the given one: matrix = matrix * Rz * Ry * Rx
 *
 * @param matrix The final matrix.
 * @param anglez The rotation angle for the z axis in degree.
 * @param angley The rotation angle for the y axis in degree.
 * @param anglex The rotation angle for the x axis in degree.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3RotateRzRyRxf(GLUSfloat matrix[9], const GLUSfloat anglez, const GLUSfloat angley, const GLUSfloat anglex);

/**
 * Creates a 4x4 matrix for scaling and multiplies it with the given one: matrix = matrix * scaleMatrix
 *
 * @param matrix The final matrix.
 * @param x The value to scale in the x axis.
 * @param y The value to scale in the y axis.
 * @param z The value to scale in the z axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Scalef(GLUSfloat matrix[16], const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a 3x3 matrix for scaling and multiplies it with the given one: matrix = matrix * scaleMatrix
 *
 * @param matrix The final matrix.
 * @param x The value to scale in the x axis.
 * @param y The value to scale in the y axis.
 * @param z The value to scale in the z axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Scalef(GLUSfloat matrix[9], const GLUSfloat x, const GLUSfloat y, const GLUSfloat z);

/**
 * Creates a 2x2 matrix for scaling and multiplies it with the given one: matrix = matrix * scaleMatrix
 *
 * @param matrix The final matrix.
 * @param x The value to scale in the x axis.
 * @param y The value to scale in the y axis.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Scalef(GLUSfloat matrix[4], const GLUSfloat x, const GLUSfloat y);

/**
 * Creates a 4x4 matrix for shearing and multiplies it with the given one: matrix = matrix * shearMatrix
 *
 * @param matrix The final matrix.
 * @param shxy Value for shearing x in y direction.
 * @param shxz Value for shearing x in z direction.
 * @param shyx Value for shearing y in x direction.
 * @param shyz Value for shearing y in z direction.
 * @param shzx Value for shearing z in x direction.
 * @param shzy Value for shearing z in y direction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4Shearf(GLUSfloat matrix[16], const GLUSfloat shxy, const GLUSfloat shxz, const GLUSfloat shyx, const GLUSfloat shyz, const GLUSfloat shzx, const GLUSfloat shzy);

/**
 * Creates a 3x3 matrix for shearing and multiplies it with the given one: matrix = matrix * shearMatrix
 *
 * @param matrix The final matrix.
 * @param shxy Value for shearing x in y direction.
 * @param shxz Value for shearing x in z direction.
 * @param shyx Value for shearing y in x direction.
 * @param shyz Value for shearing y in z direction.
 * @param shzx Value for shearing z in x direction.
 * @param shzy Value for shearing z in y direction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3Shearf(GLUSfloat matrix[9], const GLUSfloat shxy, const GLUSfloat shxz, const GLUSfloat shyx, const GLUSfloat shyz, const GLUSfloat shzx, const GLUSfloat shzy);

/**
 * Creates a 2x2 matrix for shearing and multiplies it with the given one: matrix = matrix * shearMatrix
 *
 * @param matrix The final matrix.
 * @param shx Value for shearing in x direction.
 * @param shy Value for shearing in y direction.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2Shearf(GLUSfloat matrix[4], const GLUSfloat shx, const GLUSfloat shy);

/**
 * Multiplies a 4x4 matrix with a 3D Vector.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyVector3f(GLUSfloat result[3], const GLUSfloat matrix[16], const GLUSfloat vector[3]);

/**
 * Multiplies a 4x4 matrix with a 2D Vector.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyVector2f(GLUSfloat result[2], const GLUSfloat matrix[16], const GLUSfloat vector[2]);

/**
 * Multiplies a 3x3 matrix with a 3D Vector.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3MultiplyVector3f(GLUSfloat result[3], const GLUSfloat matrix[9], const GLUSfloat vector[3]);

/**
 * Multiplies a 3x3 matrix with a 2D Vector.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3MultiplyVector2f(GLUSfloat result[2], const GLUSfloat matrix[9], const GLUSfloat vector[2]);

/**
 * Multiplies a 2x2 matrix with a 2D Vector.
 *
 * @param result The transformed vector.
 * @param matrix The matrix used for the transformation.
 * @param vector The used vector for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2MultiplyVector2f(GLUSfloat result[2], const GLUSfloat matrix[4], const GLUSfloat vector[2]);

/**
 * Multiplies a 4x4 matrix with a 3D Point, given as homogeneous coordinates.
 *
 * @param result The transformed point.
 * @param matrix The matrix used for the transformation.
 * @param point The used point for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyPoint4f(GLUSfloat result[4], const GLUSfloat matrix[16], const GLUSfloat point[4]);

/**
 * Multiplies a 3x3 matrix with a 2D Point, given as homogeneous coordinates.
 *
 * @param result The transformed point.
 * @param matrix The matrix used for the transformation.
 * @param point The used point for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3MultiplyPoint3f(GLUSfloat result[3], const GLUSfloat matrix[9], const GLUSfloat point[3]);

/**
 * Multiplies a 4x4 matrix with a plane.
 *
 * @param result The transformed plaen.
 * @param matrix The matrix used for the transformation.
 * @param plane The used plane for the transformation.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4MultiplyPlanef(GLUSfloat result[4], const GLUSfloat matrix[16], const GLUSfloat plane[4]);

/**
 * Extracts the Euler angles from a 4x4 matrix, which was created by a Rz * Rx * Ry transformation.
 *
 * @param angles Resulting anglex (Pitch), angley (Yaw) and anglez (Roll) in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4GetEulerRzRxRyf(GLUSfloat angles[3], const GLUSfloat matrix[16]);

/**
 * Extracts the Euler angles from a 4x4 matrix, which was created by a Rz * Ry * Rz transformation.
 *
 * @param angles Resulting anglex (Pitch), angley (Yaw) and anglez (Roll) in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4GetEulerRzRyRxf(GLUSfloat angles[3], const GLUSfloat matrix[16]);

/**
 * Extracts the Euler angles from a 3x3 matrix, which was created by a Rz * Rx * Ry transformation.
 *
 * @param angles Resulting anglex (Pitch), angley (Yaw) and anglez (Roll) in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3GetEulerRzRxRyf(GLUSfloat angles[3], const GLUSfloat matrix[9]);

/**
 * Extracts the Euler angles from a 3x3 matrix, which was created by a Rz * Ry * Rx transformation.
 *
 * @param angles Resulting anglex (Pitch), angley (Yaw) and anglez (Roll) in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3GetEulerRzRyRxf(GLUSfloat angles[3], const GLUSfloat matrix[9]);

/**
 * Extracts the angle from a 3x3 matrix, which is used in 2D.
 *
 * @param angle Resulting angle in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3GetAnglef(GLUSfloat* angle, const GLUSfloat matrix[9]);

/**
 * Extracts the angle from a 2x2 matrix.
 *
 * @param angle Resulting angle in degrees.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2GetAnglef(GLUSfloat* angle, const GLUSfloat matrix[4]);

/**
 * Extracts the scales from a 4x4 matrix.
 *
 * @param scales Resulting sx, sy, sz.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4GetScalef(GLUSfloat scales[3], const GLUSfloat matrix[16]);

/**
 * Extracts the scales from a 3x3 matrix.
 *
 * @param scales Resulting sx, sy, sz.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3GetScalef(GLUSfloat scales[3], const GLUSfloat matrix[9]);

/**
 * Extracts the scales from a 2x2 matrix.
 *
 * @param scales Resulting sx, sy.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix2x2GetScalef(GLUSfloat scales[2], const GLUSfloat matrix[4]);

/**
 * Extracts the translates from a 4x4 matrix.
 *
 * @param translates Resulting x, y and z.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4GetTranslatef(GLUSfloat translates[3], const GLUSfloat matrix[16]);

/**
 * Extracts the translates from a 3x3 matrix, which is used for 2D.
 *
 * @param translates Resulting x and y.
 * @param matrix The used matrix.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix3x3GetTranslatef(GLUSfloat translates[2], const GLUSfloat matrix[9]);

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

#endif /* GLUS_MATRIX_H_ */
