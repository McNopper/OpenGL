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

GLUSfloat GLUSAPIENTRY glusMathMaxf(const GLUSfloat value0, const GLUSfloat value1)
{
    return value0 > value1 ? value0 : value1;
}

GLUSfloat GLUSAPIENTRY glusMathMinf(const GLUSfloat value0, const GLUSfloat value1)
{
    return value0 < value1 ? value0 : value1;
}

GLUSfloat GLUSAPIENTRY glusMathRadToDegf(const GLUSfloat radians)
{
    return radians * 360.0f / (2.0f * GLUS_PI);
}

GLUSfloat GLUSAPIENTRY glusMathDegToRadf(const GLUSfloat degrees)
{
    return degrees * 2.0f * GLUS_PI / 360.0f;
}

GLUSfloat GLUSAPIENTRY glusMathMixf(const GLUSfloat value0, const GLUSfloat value1, const GLUSfloat t)
{
	return value0 * (1.0f - t) + value1 * t;
}

GLUSfloat GLUSAPIENTRY glusMathClampf(const GLUSfloat value, const GLUSfloat min, const GLUSfloat max)
{
	if (value < min)
	{
		return min;
	}
	if (value > max)
	{
		return max;
	}
	return value;
}

GLUSfloat GLUSAPIENTRY glusMathLengthf(const GLUSfloat x, const GLUSfloat y, const GLUSfloat z)
{
	return sqrtf(x*x + y*y + z*z);
}
