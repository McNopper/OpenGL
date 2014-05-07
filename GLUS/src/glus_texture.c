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

static GLUSvoid glusGatherSamplePoints(GLUSint sampleIndex[4], GLUSfloat sampleWeight[2], const GLUSfloat st[2], GLUSint width, GLUSint height, GLUSint stride)
{
	GLUSfloat pixelTexCoord[2];
	GLUSfloat pixelTexCoordCenter[2];

	GLUSint	  samplePixel[2];

	GLUSint i;

	for (i = 0; i < 2; i++)
	{
		pixelTexCoord[i] = glusClampf(st[i], 0.0f, 1.0f);
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

GLUSboolean GLUSAPIENTRY glusTexImage2DSampleHdrImage(GLUSfloat rgb[3], const GLUShdrimage* hdrimage, const GLUSfloat st[2])
{
	GLUSint sampleIndex[4];
	GLUSfloat sampelWeight[2];

	GLUSint i, stride;

	if (!rgb || !hdrimage || !st)
	{
		return GLUS_FALSE;
	}

	stride = 1;
	if (hdrimage->format == GLUS_RGB)
	{
		stride = 3;
	}
	else if (hdrimage->format == GLUS_RGBA)
	{
		stride = 4;
	}

	glusGatherSamplePoints(sampleIndex, sampelWeight, st, hdrimage->width, hdrimage->height, stride);

	for (i = 0; i < stride; i++)
	{
		rgb[i] = hdrimage->data[sampleIndex[0] + i] * sampelWeight[0] * sampelWeight[1];
		rgb[i] += hdrimage->data[sampleIndex[1] + i] * (1.0f - sampelWeight[0]) * sampelWeight[1];
		rgb[i] += hdrimage->data[sampleIndex[2] + i] * sampelWeight[0] * (1.0f - sampelWeight[1]);
		rgb[i] += hdrimage->data[sampleIndex[3] + i] * (1.0f - sampelWeight[0]) * (1.0f - sampelWeight[1]);
	}

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusTexImage2DSampleTgaImage(GLUSubyte rgba[4], const GLUStgaimage* tgaimage, const GLUSfloat st[2])
{
	GLUSfloat tempRGBA[4];

	GLUSint sampleIndex[4];
	GLUSfloat sampleWeight[2];

	GLUSint i, stride;

	if (!rgba || !tgaimage || !st)
	{
		return GLUS_FALSE;
	}

	stride = 1;
	if (tgaimage->format == GLUS_RGB)
	{
		stride = 3;
	}
	else if (tgaimage->format == GLUS_RGBA)
	{
		stride = 4;
	}

	glusGatherSamplePoints(sampleIndex, sampleWeight, st, tgaimage->width, tgaimage->height, stride);

	for (i = 0; i < stride; i++)
	{
		tempRGBA[i] = (GLUSfloat)tgaimage->data[sampleIndex[0] + i] * sampleWeight[0] * sampleWeight[1];
		tempRGBA[i] += (GLUSfloat)tgaimage->data[sampleIndex[1] + i] * (1.0f - sampleWeight[0]) * sampleWeight[1];
		tempRGBA[i] += (GLUSfloat)tgaimage->data[sampleIndex[2] + i] * sampleWeight[0] * (1.0f - sampleWeight[1]);
		tempRGBA[i] += (GLUSfloat)tgaimage->data[sampleIndex[3] + i] * (1.0f - sampleWeight[0]) * (1.0f - sampleWeight[1]);
	}

	// Resolve

	for (i = 0; i < 4; i++)
	{
		if (i < stride)
		{
			rgba[i] = (GLUSubyte)tempRGBA[i];
		}
		else if (i < 3)
		{
			rgba[i] = rgba[0];
		}
		else
		{
			rgba[3] = 255;
		}
	}

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusCreateTgaImage(GLUStgaimage* tgaimage, GLUSint width, GLUSint height, GLUSint depth, GLUSenum format)
{
	GLUSint stride;

	if (!tgaimage || width < 1 || height < 1 || depth < 1)
	{
		return GLUS_FALSE;
	}

	if (format == GLUS_ALPHA || format == GLUS_LUMINANCE)
	{
		stride = 1;
	}
	else if (format == GLUS_RGB)
	{
		stride = 3;
	}
	else if (format == GLUS_RGBA)
	{
		stride = 4;
	}
	else
	{
		return GLUS_FALSE;
	}

	tgaimage->data = (GLUSubyte*)malloc(width * height * depth * stride * sizeof(GLUSubyte));
	if (!tgaimage->data)
	{
		return GLUS_FALSE;
	}
	tgaimage->width = width;
	tgaimage->height = height;
	tgaimage->depth = depth;
	tgaimage->format = format;

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusCreateHdrImage(GLUShdrimage* hdrimage, GLUSint width, GLUSint height, GLUSint depth, GLUSenum format)
{
	GLUSint stride;

	if (!hdrimage || width < 1 || height < 1 || depth < 1)
	{
		return GLUS_FALSE;
	}

	if (format == GLUS_ALPHA || format == GLUS_LUMINANCE)
	{
		stride = 1;
	}
	else if (format == GLUS_RGB)
	{
		stride = 3;
	}
	else if (format == GLUS_RGBA)
	{
		stride = 4;
	}
	else
	{
		return GLUS_FALSE;
	}

	hdrimage->data = (GLUSfloat*)malloc(width * height * depth * stride * sizeof(GLUSfloat));
	if (!hdrimage->data)
	{
		return GLUS_FALSE;
	}
	hdrimage->width = width;
	hdrimage->height = height;
	hdrimage->depth = depth;
	hdrimage->format = format;

	return GLUS_TRUE;
}
