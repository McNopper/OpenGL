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

GLUSvoid GLUSAPIENTRY glusQuaternionIdentityf(GLUSfloat quaternion[4])
{
    quaternion[0] = 0.0f;
    quaternion[1] = 0.0f;
    quaternion[2] = 0.0f;
    quaternion[3] = 1.0f;
}

GLUSvoid GLUSAPIENTRY glusQuaternionCopyf(GLUSfloat result[4], const GLUSfloat quaternion[4])
{
    result[0] = quaternion[0];
    result[1] = quaternion[1];
    result[2] = quaternion[2];
    result[3] = quaternion[3];
}

GLUSfloat GLUSAPIENTRY glusQuaternionNormf(const GLUSfloat quaternion[4])
{
    GLUSfloat x = quaternion[0];
    GLUSfloat y = quaternion[1];
    GLUSfloat z = quaternion[2];
    GLUSfloat w = quaternion[3];

    return sqrtf(x * x + y * y + z * z + w * w);
}

GLUSboolean GLUSAPIENTRY glusQuaternionNormalizef(GLUSfloat quaternion[4])
{
    GLUSint i;

    GLUSfloat norm = glusQuaternionNormf(quaternion);

    if (norm == 0.0f)
    {
        return GLUS_FALSE;
    }

    for (i = 0; i < 4; i++)
    {
        quaternion[i] /= norm;
    }

    return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusQuaternionAddQuaternionf(GLUSfloat result[4], const GLUSfloat quaternion0[4], const GLUSfloat quaternion1[4])
{
    GLUSint i;

    for (i = 0; i < 4; i++)
    {
        result[i] = quaternion0[i] + quaternion1[i];
    }
}

GLUSvoid GLUSAPIENTRY glusQuaternionSubQuaternionf(GLUSfloat result[4], const GLUSfloat quaternion0[4], const GLUSfloat quaternion1[4])
{
    GLUSint i;

    for (i = 0; i < 4; i++)
    {
        result[i] = quaternion0[i] - quaternion1[i];
    }
}

GLUSvoid GLUSAPIENTRY glusQuaternionMultiplyQuaternionf(GLUSfloat result[4], const GLUSfloat quaternion0[4], const GLUSfloat quaternion1[4])
{
    GLUSfloat temp[4];

    temp[0] = quaternion0[3] * quaternion1[0] + quaternion0[0] * quaternion1[3] + quaternion0[1] * quaternion1[2] - quaternion0[2] * quaternion1[1];
    temp[1] = quaternion0[3] * quaternion1[1] - quaternion0[0] * quaternion1[2] + quaternion0[1] * quaternion1[3] + quaternion0[2] * quaternion1[0];
    temp[2] = quaternion0[3] * quaternion1[2] + quaternion0[0] * quaternion1[1] - quaternion0[1] * quaternion1[0] + quaternion0[2] * quaternion1[3];
    temp[3] = quaternion0[3] * quaternion1[3] - quaternion0[0] * quaternion1[0] - quaternion0[1] * quaternion1[1] - quaternion0[2] * quaternion1[2];

    glusQuaternionCopyf(result, temp);
}

GLUSvoid GLUSAPIENTRY glusQuaternionConjugatef(GLUSfloat quaternion[4])
{
    quaternion[0] = -quaternion[0];
    quaternion[1] = -quaternion[1];
    quaternion[2] = -quaternion[2];
    // w remains the same
}

GLUSboolean GLUSAPIENTRY glusQuaternionInversef(GLUSfloat quaternion[4])
{
    GLUSint i;

    GLUSfloat norm = glusQuaternionNormf(quaternion);

    if (norm == 0.0f)
    {
        return GLUS_FALSE;
    }

    glusQuaternionConjugatef(quaternion);

    for (i = 0; i < 4; i++)
    {
        quaternion[i] /= (norm * norm);
    }

    return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusQuaternionInverseUnitf(GLUSfloat quaternion[4])
{
    glusQuaternionConjugatef(quaternion);
}

GLUSvoid GLUSAPIENTRY glusQuaternionRotatef(GLUSfloat quaternion[4], const GLUSfloat angle, const GLUSfloat x, const GLUSfloat y, const GLUSfloat z)
{
    GLUSfloat halfAngleRadian = glusDegToRadf(angle) * 0.5f;

    GLUSfloat s = sinf(halfAngleRadian);

    GLUSfloat normalized[3] = { x, y, z };

    glusVector3Normalizef(normalized);

    quaternion[0] = normalized[0] * s;
    quaternion[1] = normalized[1] * s;
    quaternion[2] = normalized[2] * s;
    quaternion[3] = cosf(halfAngleRadian);
}

GLUSvoid GLUSAPIENTRY glusQuaternionRotateRxf(GLUSfloat quaternion[4], const GLUSfloat angle)
{
    GLUSfloat halfAngleRadian = glusDegToRadf(angle) * 0.5f;

    quaternion[0] = sinf(halfAngleRadian);
    quaternion[1] = 0.0f;
    quaternion[2] = 0.0f;
    quaternion[3] = cosf(halfAngleRadian);
}

GLUSvoid GLUSAPIENTRY glusQuaternionRotateRyf(GLUSfloat quaternion[4], const GLUSfloat angle)
{
    GLUSfloat halfAngleRadian = glusDegToRadf(angle) * 0.5f;

    quaternion[0] = 0.0f;
    quaternion[1] = sinf(halfAngleRadian);
    quaternion[2] = 0.0f;
    quaternion[3] = cosf(halfAngleRadian);
}

GLUSvoid GLUSAPIENTRY glusQuaternionRotateRzf(GLUSfloat quaternion[4], const GLUSfloat angle)
{
    GLUSfloat halfAngleRadian = glusDegToRadf(angle) * 0.5f;

    quaternion[0] = 0.0f;
    quaternion[1] = 0.0f;
    quaternion[2] = sinf(halfAngleRadian);
    quaternion[3] = cosf(halfAngleRadian);
}

GLUSvoid GLUSAPIENTRY glusQuaternionRotateRzRxRyf(GLUSfloat quaternion[4], const GLUSfloat anglez, const GLUSfloat anglex, const GLUSfloat angley)
{
	GLUSfloat rotZ[4];
	GLUSfloat rotX[4];
	GLUSfloat rotY[4];

    glusQuaternionRotateRzf(rotZ, anglez);
    glusQuaternionRotateRxf(rotX, anglex);
	glusQuaternionRotateRyf(rotY, angley);

	glusQuaternionMultiplyQuaternionf(quaternion, rotZ, rotX);
	glusQuaternionMultiplyQuaternionf(quaternion, quaternion, rotY);
}

GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionRotateRzRyRxf(GLUSfloat quaternion[4], const GLUSfloat anglez, const GLUSfloat angley, const GLUSfloat anglex)
{
	GLUSfloat rotZ[4];
	GLUSfloat rotY[4];
	GLUSfloat rotX[4];

    glusQuaternionRotateRzf(rotZ, anglez);
	glusQuaternionRotateRyf(rotY, angley);
    glusQuaternionRotateRxf(rotX, anglex);

	glusQuaternionMultiplyQuaternionf(quaternion, rotZ, rotY);
	glusQuaternionMultiplyQuaternionf(quaternion, quaternion, rotX);
}

GLUSvoid GLUSAPIENTRY glusQuaternionGetMatrix4x4f(GLUSfloat matrix[16], const GLUSfloat quaternion[4])
{
    GLUSfloat x = quaternion[0];
    GLUSfloat y = quaternion[1];
    GLUSfloat z = quaternion[2];
    GLUSfloat w = quaternion[3];

    matrix[0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
    matrix[1] = 2.0f * x * y + 2.0f * w * z;
    matrix[2] = 2.0f * x * z - 2.0f * w * y;
    matrix[3] = 0.0f;

    matrix[4] = 2.0f * x * y - 2.0f * w * z;
    matrix[5] = 1.0f - 2.0f * x * x - 2.0f * z * z;
    matrix[6] = 2.0f * y * z + 2.0f * w * x;
    matrix[7] = 0.0f;

    matrix[8] = 2.0f * x * z + 2.0f * w * y;
    matrix[9] = 2.0f * y * z - 2.0f * w * x;
    matrix[10] = 1.0f - 2.0f * x * x - 2.0f * y * y;
    matrix[11] = 0.0f;

    matrix[12] = 0.0f;
    matrix[13] = 0.0f;
    matrix[14] = 0.0f;
    matrix[15] = 1.0f;
}

GLUSvoid GLUSAPIENTRY glusQuaternionGetMatrix3x3f(GLUSfloat matrix[9], const GLUSfloat quaternion[4])
{
    GLUSfloat x = quaternion[0];
    GLUSfloat y = quaternion[1];
    GLUSfloat z = quaternion[2];
    GLUSfloat w = quaternion[3];

    matrix[0] = 1.0f - 2.0f * y * y - 2.0f * z * z;
    matrix[1] = 2.0f * x * y + 2.0f * w * z;
    matrix[2] = 2.0f * x * z - 2.0f * w * y;

    matrix[3] = 2.0f * x * y - 2.0f * w * z;
    matrix[4] = 1.0f - 2.0f * x * x - 2.0f * z * z;
    matrix[5] = 2.0f * y * z + 2.0f * w * x;

    matrix[6] = 2.0f * x * z + 2.0f * w * y;
    matrix[7] = 2.0f * y * z - 2.0f * w * x;
    matrix[8] = 1.0f - 2.0f * x * x - 2.0f * y * y;
}

GLUSboolean GLUSAPIENTRY glusQuaternionGetPoint4f(GLUSfloat point[4], const GLUSfloat quaternion[4])
{
	if (quaternion[3] != 0.0f)
	{
		return GLUS_FALSE;
	}

	point[0] = quaternion[0];
	point[1] = quaternion[1];
	point[2] = quaternion[2];
	point[3] = 1.0f;

	return GLUS_TRUE;
}

GLUSAPI GLUSboolean GLUSAPIENTRY glusQuaternionGetVector3f(GLUSfloat vector[3], const GLUSfloat quaternion[4])
{
	if (quaternion[3] != 0.0f)
	{
		return GLUS_FALSE;
	}

	vector[0] = quaternion[0];
	vector[1] = quaternion[1];
	vector[2] = quaternion[2];

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusQuaternionGetEulerRzRxRyf(GLUSfloat angles[3], const GLUSfloat quaternion[4])
{
    GLUSfloat x = quaternion[0];
    GLUSfloat y = quaternion[1];
    GLUSfloat z = quaternion[2];
    GLUSfloat w = quaternion[3];

    GLUSfloat f11 = 1.0f - 2.0f * x * x - 2.0f * z * z;
    GLUSfloat f21 = 2.0f * y * z + 2.0f * w * x;

    if (f11 != 0.0f)
    {
        GLUSfloat f20 = 2.0f * x * z - 2.0f * w * y;
        GLUSfloat f22 = 1.0f - 2.0f * x * x - 2.0f * y * y;
        GLUSfloat f01 = 2.0f * x * y - 2.0f * w * z;

        angles[0] = glusRadToDegf(asinf(f21));
        angles[1] = glusRadToDegf(atan2f(-f20, f22));
        angles[2] = glusRadToDegf(atan2f(-f01, f11));
    }
    else
    {
        GLUSfloat f10 = 2.0f * x * y + 2.0f * w * z;
        GLUSfloat f00 = 1.0f - 2.0f * y * y - 2.0f * z * z;

        angles[0] = glusRadToDegf(asinf(f21));
        angles[1] = 0.0f;
        angles[2] = glusRadToDegf(atan2f(f10, f00));
    }
}

GLUSAPI GLUSvoid GLUSAPIENTRY glusQuaternionGetEulerRzRyRxf(GLUSfloat angles[3], const GLUSfloat quaternion[4])
{
    GLUSfloat x = quaternion[0];
    GLUSfloat y = quaternion[1];
    GLUSfloat z = quaternion[2];
    GLUSfloat w = quaternion[3];

    GLUSfloat f00 = 1.0f - 2.0f * y * y - 2.0f * z * z;
    GLUSfloat f20 = 2.0f * x * z - 2.0f * w * y;

    if (f00 != 0.0f)
    {
    	GLUSfloat f10 = 2.0f * x * y + 2.0f * w * z;
    	GLUSfloat f21 = 2.0f * y * z + 2.0f * w * x;
    	GLUSfloat f22 = 1.0f - 2.0f * x * x - 2.0f * y * y;

        angles[0] = glusRadToDegf(atan2f(f21, f22));
    	angles[1] = glusRadToDegf(asinf(-f20));
    	angles[2] = glusRadToDegf(atan2f(f10, f00));
    }
    else
    {
    	GLUSfloat f01 = 2.0f * x * y - 2.0f * w * z;
    	GLUSfloat f11 = 1.0f - 2.0f * x * x - 2.0f * z * z;

        angles[0] = glusRadToDegf(atan2f(f01, f11));
    	angles[1] = glusRadToDegf(asinf(-f20));
    	angles[2] = 0.0f;
    }

}

GLUSvoid GLUSAPIENTRY glusQuaternionSlerpf(GLUSfloat result[4], const GLUSfloat quaternion0[4], const GLUSfloat quaternion1[4], const GLUSfloat t)
{
    GLUSint i;

    GLUSfloat cosAlpha = quaternion0[0] * quaternion1[0] + quaternion0[1] * quaternion1[1] + quaternion0[2] * quaternion1[2] + quaternion0[3] * quaternion1[3];

    GLUSfloat sinAlpha = sqrtf(1.0f - cosAlpha * cosAlpha);

    GLUSfloat alpha = acosf(cosAlpha);

    GLUSfloat a = sinf(alpha * (1.0f - t)) / sinAlpha;

    GLUSfloat b = sinf(alpha * t) / sinAlpha;

    for (i = 0; i < 4; i++)
    {
        result[i] = a * quaternion0[i] + b * quaternion1[i];
    }
}
