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

GLUSvoid _glusImageGatherSamplePoints(GLUSint sampleIndex[4], GLUSfloat sampleWeight[2], const GLUSfloat st[2], GLUSint width, GLUSint height, GLUSint stride)
{
	GLUSfloat pixelTexCoord[2];
	GLUSfloat pixelTexCoordCenter[2];

	GLUSint	  samplePixel[2];

	GLUSint i;

	for (i = 0; i < 2; i++)
	{
		pixelTexCoord[i] = glusMathClampf(st[i], 0.0f, 1.0f);
	}

	pixelTexCoord[0] = pixelTexCoord[0] * (GLUSfloat)width;
	pixelTexCoord[1] = pixelTexCoord[1] * (GLUSfloat)height;

	for (i = 0; i < 2; i++)
	{
		pixelTexCoordCenter[i] = floorf(pixelTexCoord[i]) + 0.5f;
	}

	for (i = 0; i < 2; i++)
	{
		sampleWeight[i] = 1.0f - fabsf(pixelTexCoordCenter[i] - pixelTexCoord[i]);
	}

	// Check, if in bounds.

	samplePixel[0] = (GLUSint)pixelTexCoord[0];
	if (samplePixel[0] == width)
	{
		samplePixel[0]--;
	}

	samplePixel[1] = (GLUSint)pixelTexCoord[1];
	if (samplePixel[1] == height)
	{
		samplePixel[1]--;
	}

	// Main sample point

	sampleIndex[0] = samplePixel[1] * width * stride + samplePixel[0] * stride;

	// s axis sample point

	if (pixelTexCoordCenter[0] > 0.5f && pixelTexCoord[0] < pixelTexCoordCenter[0])
	{
		sampleIndex[1] = sampleIndex[0] - stride;
		sampleIndex[3] = -stride;
	}
	else if (pixelTexCoordCenter[0] < (GLUSfloat)width - 0.5f && pixelTexCoord[0] > pixelTexCoordCenter[0])
	{
		sampleIndex[1] = sampleIndex[0] + stride;
		sampleIndex[3] = stride;
	}
	else
	{
		sampleIndex[1] = sampleIndex[0];
		sampleIndex[3] = 0;
	}

	// t axis sample point

	if (pixelTexCoordCenter[1] > 0.5f && pixelTexCoord[1] < pixelTexCoordCenter[1])
	{
		sampleIndex[2] = sampleIndex[0] - stride * width;
	}
	else if (pixelTexCoordCenter[1] < (GLUSfloat)height - 0.5f && pixelTexCoord[1] > pixelTexCoordCenter[1])
	{
		sampleIndex[2] = sampleIndex[0] + stride * width;
	}
	else
	{
		sampleIndex[2] = sampleIndex[0];
	}

	// diagonal sample point

	sampleIndex[3] += sampleIndex[2];
}
