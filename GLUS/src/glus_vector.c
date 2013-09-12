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

GLUSvoid GLUSAPIENTRY glusVector3Copyf(GLUSfloat result[3], const GLUSfloat vector[3])
{
	result[0] = vector[0];
	result[1] = vector[1];
	result[2] = vector[2];
}

GLUSvoid GLUSAPIENTRY glusVector2Copyf(GLUSfloat result[2], const GLUSfloat vector[2])
{
	result[0] = vector[0];
	result[1] = vector[1];
}

GLUSvoid GLUSAPIENTRY glusVector3AddVector3f(GLUSfloat result[3], const GLUSfloat vector0[3], const GLUSfloat vector1[3])
{
	result[0] = vector0[0] + vector1[0];
	result[1] = vector0[1] + vector1[1];
	result[2] = vector0[2] + vector1[2];
}

GLUSvoid GLUSAPIENTRY glusVector2AddVector2f(GLUSfloat result[2], const GLUSfloat vector0[2], const GLUSfloat vector1[2])
{
	result[0] = vector0[0] + vector1[0];
	result[1] = vector0[1] + vector1[1];
}

GLUSvoid GLUSAPIENTRY glusVector3SubtractVector3f(GLUSfloat result[3], const GLUSfloat vector0[3], const GLUSfloat vector1[3])
{
	result[0] = vector0[0] - vector1[0];
	result[1] = vector0[1] - vector1[1];
	result[2] = vector0[2] - vector1[2];
}

GLUSvoid GLUSAPIENTRY glusVector2SubtractVector2f(GLUSfloat result[2], const GLUSfloat vector0[2], const GLUSfloat vector1[2])
{
	result[0] = vector0[0] - vector1[0];
	result[1] = vector0[1] - vector1[1];
}

GLUSvoid GLUSAPIENTRY glusVector3MultiplyScalarf(GLUSfloat result[3], const GLUSfloat vector[3], const GLUSfloat scalar)
{
	result[0] = vector[0] * scalar;
	result[1] = vector[1] * scalar;
	result[2] = vector[2] * scalar;
}

GLUSvoid GLUSAPIENTRY glusVector2MultiplyScalarf(GLUSfloat result[2], const GLUSfloat vector[2], const GLUSfloat scalar)
{
	result[0] = vector[0] * scalar;
	result[1] = vector[1] * scalar;
}

GLUSfloat GLUSAPIENTRY glusVector3Lengthf(const GLUSfloat vector[3])
{
	return sqrtf(vector[0] * vector[0] + vector[1] * vector[1] + vector[2] * vector[2]);
}

GLUSfloat GLUSAPIENTRY glusVector2Lengthf(const GLUSfloat vector[2])
{
	return sqrtf(vector[0] * vector[0] + vector[1] * vector[1]);
}

GLUSboolean GLUSAPIENTRY glusVector3Normalizef(GLUSfloat vector[3])
{
	GLUSint i;

	GLUSfloat length = glusVector3Lengthf(vector);

	if (length == 0.0f)
	{
		return GLUS_FALSE;
	}

	for (i = 0; i < 3; i++)
	{
		vector[i] /= length;
	}

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusVector2Normalizef(GLUSfloat vector[2])
{
	GLUSint i;

	GLUSfloat length = glusVector2Lengthf(vector);

	if (length == 0.0f)
	{
		return GLUS_FALSE;
	}

	for (i = 0; i < 2; i++)
	{
		vector[i] /= length;
	}

	return GLUS_TRUE;
}

GLUSfloat GLUSAPIENTRY glusVector3Dotf(const GLUSfloat vector0[3], const GLUSfloat vector1[3])
{
	return vector0[0] * vector1[0] + vector0[1] * vector1[1] + vector0[2] * vector1[2];
}

GLUSfloat GLUSAPIENTRY glusVector2Dotf(const GLUSfloat vector0[2], const GLUSfloat vector1[2])
{
	return vector0[0] * vector1[0] + vector0[1] * vector1[1];
}

GLUSvoid GLUSAPIENTRY glusVector3Crossf(GLUSfloat result[3], const GLUSfloat vector0[3], const GLUSfloat vector1[3])
{
	GLUSint i;

	GLUSfloat temp[3];

	temp[0] = vector0[1] * vector1[2] - vector0[2] * vector1[1];
	temp[1] = vector0[2] * vector1[0] - vector0[0] * vector1[2];
	temp[2] = vector0[0] * vector1[1] - vector0[1] * vector1[0];

	for (i = 0; i < 3; i++)
	{
		result[i] = temp[i];
	}
}

GLUSvoid GLUSAPIENTRY glusVector3GetQuaternionf(GLUSfloat result[4], const GLUSfloat vector[3])
{
	result[0] = vector[0];
	result[1] = vector[1];
	result[2] = vector[2];
	result[3] = 0.0f;
}

GLUSvoid GLUSAPIENTRY glusVector3GetPoint4f(GLUSfloat result[4], const GLUSfloat vector[3])
{
	result[0] = vector[0];
	result[1] = vector[1];
	result[2] = vector[2];
	result[3] = 1.0f;
}

GLUSvoid GLUSAPIENTRY glusVector2GetPoint3f(GLUSfloat result[3], const GLUSfloat vector[2])
{
	result[0] = vector[0];
	result[1] = vector[1];
	result[2] = 1.0f;
}

GLUSboolean GLUSAPIENTRY glusVector3GramSchmidtOrthof(GLUSfloat result[3], const GLUSfloat u[3], const GLUSfloat v[3])
{
	GLUSfloat uProjV[3];

	GLUSfloat vDotU;

	GLUSfloat uDotU = glusVector3Dotf(u, u);

	if (uDotU == 0.0f)
	{
		return GLUS_FALSE;
	}

	vDotU = glusVector3Dotf(v, u);

	uProjV[0] = u[0] * vDotU / uDotU;
	uProjV[1] = u[1] * vDotU / uDotU;
	uProjV[2] = u[2] * vDotU / uDotU;

	glusVector3SubtractVector3f(result, v, uProjV);

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusVector2GramSchmidtOrthof(GLUSfloat result[2], const GLUSfloat u[2], const GLUSfloat v[2])
{
	GLUSfloat uProjV[2];

	GLUSfloat vDotU;

	GLUSfloat uDotU = glusVector2Dotf(u, u);

	if (uDotU == 0.0f)
	{
		return GLUS_FALSE;
	}

	vDotU = glusVector2Dotf(v, u);

	uProjV[0] = u[0] * vDotU / uDotU;
	uProjV[1] = u[1] * vDotU / uDotU;

	glusVector2SubtractVector2f(result, v, uProjV);

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusVector3Reflectf(GLUSfloat result[3], const GLUSfloat incident[3], const GLUSfloat normal[3])
{
	glusVector3MultiplyScalarf(result, normal, 2.0f * glusVector3Dotf(normal, incident));

	glusVector3SubtractVector3f(result, incident, result);
}

GLUSvoid GLUSAPIENTRY glusVector2Reflectf(GLUSfloat result[2], const GLUSfloat incident[2], const GLUSfloat normal[2])
{
	glusVector2MultiplyScalarf(result, normal, 2.0f * glusVector3Dotf(normal, incident));

	glusVector2SubtractVector2f(result, incident, result);
}

GLUSvoid GLUSAPIENTRY glusVector3Refractf(GLUSfloat result[3], const GLUSfloat incident[3], const GLUSfloat normal[3], const float eta)
{
	// see http://www.opengl.org/sdk/docs/manglsl/xhtml/refract.xml
	// see http://en.wikipedia.org/wiki/Snell%27s_law
	// see http://www.hugi.scene.org/online/coding/hugi%2023%20-%20torefrac.htm for a and b vector.
	// In this implementation, the incident vector points into the interface. So the sings do change.

	GLUSfloat nDotI = glusVector3Dotf(normal, incident);

	GLUSfloat k = 1.0f - eta * eta * (1.0f - nDotI * nDotI);

	if (k < 0.0f)
	{
		result[0] = 0.0f;
		result[1] = 0.0f;
		result[2] = 0.0f;
	}
	else
	{
		GLUSfloat a[3];
		GLUSfloat b[3];

		glusVector3MultiplyScalarf(a, incident, eta);

		glusVector3MultiplyScalarf(b, normal, eta * nDotI + sqrtf(k));

		glusVector3SubtractVector3f(result, a, b);
	}
}

GLUSvoid GLUSAPIENTRY glusVector2Refractf(GLUSfloat result[2], const GLUSfloat incident[2], const GLUSfloat normal[2], const float eta)
{
	GLUSfloat nDotI = glusVector2Dotf(normal, incident);

	GLUSfloat k = 1.0f - eta * eta * (1.0f - nDotI * nDotI);

	if (k < 0.0f)
	{
		result[0] = 0.0f;
		result[1] = 0.0f;
	}
	else
	{
		GLUSfloat a[2];
		GLUSfloat b[2];

		glusVector2MultiplyScalarf(a, incident, eta);

		glusVector2MultiplyScalarf(b, normal, eta * nDotI + sqrtf(k));

		glusVector2SubtractVector2f(result, a, b);
	}
}

GLUSfloat GLUSAPIENTRY glusVector3Fresnelf(const GLUSfloat incident[3], const GLUSfloat normal[3], const GLUSfloat R0)
{
	// see http://en.wikipedia.org/wiki/Schlick%27s_approximation

	GLUSfloat negIncident[3];

	glusVector3MultiplyScalarf(negIncident, incident, -1.0f);

	return R0 + (1.0f - R0) * powf(1.0f - glusVector3Dotf(negIncident, normal), 5.0f);
}

GLUSfloat GLUSAPIENTRY glusVector2Fresnelf(const GLUSfloat incident[2], const GLUSfloat normal[2], const GLUSfloat R0)
{
	GLUSfloat negIncident[2];

	glusVector2MultiplyScalarf(negIncident, incident, -1.0f);

	return R0 + (1.0f - R0) * powf(1.0f - glusVector2Dotf(negIncident, normal), 5.0f);
}
