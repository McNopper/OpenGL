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

#define GLUS_UNIFORM_RANDOM_BIAS 0.0000001f

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

	// Avoid logf(0.0f) and logf(1.0f)
	x1 = glusRandomUniformGetFloatf(GLUS_UNIFORM_RANDOM_BIAS, 1.0f - GLUS_UNIFORM_RANDOM_BIAS);
	x2 = glusRandomUniformGetFloatf(0.0f, 1.0f);

	return mean + standardDeviation * (sqrtf(-2.0f * logf(x1)) * cosf(2.0f * GLUS_PI * x2));
}

// see http://mathworld.wolfram.com/HammersleyPointSet.html
// see https://github.com/wdas/brdf/blob/master/src/shaderTemplates/brdfIBL.frag

GLUSboolean GLUSAPIENTRY glusRandomHammersleyf(GLUSfloat result[2], const GLUSuint sample, const GLUSubyte m)
{
	GLUSuint revertSample;
	GLUSfloat binaryFractionFactor;

	if (!result)
	{
		return GLUS_FALSE;
	}

	// Check, if m is in the allowed range, as only 32bit unsigned integer is supported.
	if (m == 0 || m > 32)
	{
		return GLUS_FALSE;
	}

	// If not all bits are used: Check, if sample is out of bounds.
	if (m < 32 && sample >= (GLUSuint)(1 << m))
	{
		return GLUS_FALSE;
	}

	// Revert bits by swapping blockwise. Lower bits are moved up and higher bits down.
	revertSample = (sample << 16u) | (sample >> 16u);
	revertSample = ((revertSample & 0x00ff00ffu) << 8u) | ((revertSample & 0xff00ff00u) >> 8u);
	revertSample = ((revertSample & 0x0f0f0f0fu) << 4u) | ((revertSample & 0xf0f0f0f0u) >> 4u);
	revertSample = ((revertSample & 0x33333333u) << 2u) | ((revertSample & 0xccccccccu) >> 2u);
	revertSample = ((revertSample & 0x55555555u) << 1u) | ((revertSample & 0xaaaaaaaau) >> 1u);

	// Shift back, as only m bits are used.
	revertSample = revertSample >> (32 - m);

	// Results are in range [0.0 1.0] and not [0.0, 1.0[.
	binaryFractionFactor = 1.0f / (powf(2.0f, (GLUSfloat)m) - 1.0f);

	result[0] = (GLUSfloat)revertSample * binaryFractionFactor;
	result[1] = (GLUSfloat)sample * binaryFractionFactor;

	return GLUS_TRUE;
}
