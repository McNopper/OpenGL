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

GLUSvoid GLUSAPIENTRY glusRandomSetSeed(const GLUSuint seed)
{
	srand(seed);
}

GLUSfloat GLUSAPIENTRY glusRandomUniformGetFloatf(const GLUSfloat start, const GLUSfloat end)
{
	return ((GLUSfloat)rand() / (GLUSfloat)RAND_MAX) * (end - start) + start;
}

// see http://mathworld.wolfram.com/Box-MullerTransformation.html

GLUSfloat GLUSAPIENTRY glusRandomNormalGetFloatf(const GLUSfloat mean, const GLUSfloat standardDeviation)
{
	GLUSfloat x1, x2;

	x1 = glusRandomUniformGetFloatf(0.0f, 1.0f);
	x2 = glusRandomUniformGetFloatf(0.0f, 1.0f);

	return mean + standardDeviation * (sqrtf(-2.0f * logf(x1)) * cosf(2.0f * GLUS_PI * x2));
}
