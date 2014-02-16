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

static GLUSboolean glusCheckFileWrite(FILE* f, size_t actualWrite, size_t expectedWrite)
{
	if (!f)
	{
		return GLUS_FALSE;
	}

	if (actualWrite < expectedWrite)
	{
		if (ferror(f))
		{
			fclose(f);

			return GLUS_FALSE;
		}
	}

	return GLUS_TRUE;
}

static GLUSvoid glusSwapColorChannel(GLUSint width, GLUSint height, GLUSenum format, GLUSubyte* data)
{
	GLUSint i;
	GLUSubyte temp;
	GLUSint bytesPerPixel = 3;

	if (!data)
	{
		return;
	}

	if (format == GLUS_RGBA)
	{
		bytesPerPixel = 4;
	}

	// swap the R and B values to get RGB since the bitmap color format is in BGR
	for (i = 0; i < width * height * bytesPerPixel; i += bytesPerPixel)
	{
		temp = data[i];
		data[i] = data[i + 2];
		data[i + 2] = temp;
	}
}

GLUSboolean GLUSAPIENTRY glusSaveTgaImage(const GLUSchar* filename, const GLUStgaimage* tgaimage)
{
	FILE* file;
	GLUSubyte buffer[12];
	GLUSubyte bitsPerPixel;
	size_t elementsWritten;
	GLUSubyte* data;

	// check, if we have a valid pointer
	if (!filename || !tgaimage)
	{
		return GLUS_FALSE;
	}

	// open filename in "write binary" mode
	file = fopen(filename, "wb");

	if (!file)
	{
		return GLUS_FALSE;
	}

	switch (tgaimage->format)
	{
		case GLUS_RED:
		case GLUS_ALPHA:
		case GLUS_LUMINANCE:
			bitsPerPixel = 8;
		break;
		case GLUS_RGB:
			bitsPerPixel = 24;
		break;
		case GLUS_RGBA:
			bitsPerPixel = 32;
		break;
		default:
			fclose(file);
			return GLUS_FALSE;
	}

	if (bitsPerPixel == 8)
	{
		buffer[2] = 3;
	}
	else
	{
		buffer[2] = 2;
	}

	// TGA header
	buffer[0] = 0;
	buffer[1] = 0;

	buffer[3] = 0;
	buffer[4] = 0;
	buffer[5] = 0;
	buffer[6] = 0;
	buffer[7] = 0;
	buffer[8] = 0;
	buffer[9] = 0;
	buffer[10] = 0;
	buffer[11] = 0;

	elementsWritten = fwrite(buffer, 1, 12, file);

	if (!glusCheckFileWrite(file, elementsWritten, 12))
	{
		return GLUS_FALSE;
	}

	elementsWritten = fwrite(&tgaimage->width, 1, sizeof(tgaimage->width), file);

	if (!glusCheckFileWrite(file, elementsWritten, 2))
	{
		return GLUS_FALSE;
	}

	elementsWritten = fwrite(&tgaimage->height, 1, sizeof(tgaimage->height), file);

	if (!glusCheckFileWrite(file, elementsWritten, 2))
	{
		return GLUS_FALSE;
	}

	elementsWritten = fwrite(&bitsPerPixel, 1, sizeof(bitsPerPixel), file);

	if (!glusCheckFileWrite(file, elementsWritten, 1))
	{
		return GLUS_FALSE;
	}

	buffer[0] = 0;

	elementsWritten = fwrite(buffer, 1, 1, file);

	if (!glusCheckFileWrite(file, elementsWritten, 1))
	{
		return GLUS_FALSE;
	}

	data = malloc(tgaimage->width * tgaimage->height * bitsPerPixel / 8);

	if (!data)
	{
		fclose(file);

		return GLUS_FALSE;
	}

	memcpy(data, tgaimage->data, tgaimage->width * tgaimage->height * bitsPerPixel / 8);

	if (bitsPerPixel >= 24)
	{
		glusSwapColorChannel(tgaimage->width, tgaimage->height, tgaimage->format, data);
	}

	elementsWritten = fwrite(data, 1, tgaimage->width * tgaimage->height * bitsPerPixel / 8, file);

	free(data);

	if (!glusCheckFileWrite(file, elementsWritten, tgaimage->width * tgaimage->height * bitsPerPixel / 8))
	{
		return GLUS_FALSE;
	}

	fclose(file);

	return GLUS_TRUE;
}

static GLUSvoid convertRGB(GLUSubyte* rgbe, const GLUSfloat* rgb)
{
	GLUSfloat significant[3];
	GLUSint exponent[3];
	GLUSint maxExponent;

	significant[0] = frexpf(rgb[0], &exponent[0]);
	significant[1] = frexpf(rgb[1], &exponent[1]);
	significant[2] = frexpf(rgb[2], &exponent[2]);

	maxExponent = exponent[0];
	if (exponent[1] > maxExponent)
	{
		maxExponent = exponent[1];
	}
	if (exponent[2] > maxExponent)
	{
		maxExponent = exponent[2];
	}

	rgbe[0] = (GLUSubyte)(significant[0] * 256.0f * powf(2.0f, (GLUSfloat)(exponent[0] - maxExponent)));
	rgbe[1] = (GLUSubyte)(significant[1] * 256.0f * powf(2.0f, (GLUSfloat)(exponent[1] - maxExponent)));
	rgbe[2] = (GLUSubyte)(significant[2] * 256.0f * powf(2.0f, (GLUSfloat)(exponent[2] - maxExponent)));
	rgbe[3] = (GLUSubyte)(maxExponent + 128);
}

GLUSboolean GLUSAPIENTRY glusSaveHdrImage(const GLUSchar* filename, const GLUShdrimage* hdrimage)
{
	FILE* file;
	size_t elementsWritten;
	GLUSubyte rgbe[4];
	GLUSint x, y;

	// check, if we have a valid pointer
	if (!filename || !hdrimage)
	{
		return GLUS_FALSE;
	}

	// open filename in "write binary" mode
	file = fopen(filename, "wb");

	if (!file)
	{
		return GLUS_FALSE;
	}

	// Header
	elementsWritten = fputs("#?RADIANCE\n#Saved with GLUS\nFORMAT=32-bit_rle_rgbe\n\n", file);

	if (!glusCheckFileWrite(file, elementsWritten, 52))
	{
		return GLUS_FALSE;
	}

	// Resolution
	if (fprintf(file, "-Y %d +X %d\n", hdrimage->height, hdrimage->width) < 0)
	{
		fclose(file);

		return GLUS_FALSE;
	}

	// Non compressed data
	for (y = hdrimage->height - 1; y >= 0; y--)
	{
		for (x = 0; x < hdrimage->width; x++)
		{
			convertRGB(rgbe, &hdrimage->data[(y * hdrimage->width + x) * 3]);

			elementsWritten = fwrite(rgbe, 1, 4 * sizeof(GLUSubyte), file);

			if (!glusCheckFileWrite(file, elementsWritten, 4 * sizeof(GLUSubyte)))
			{
				return GLUS_FALSE;
			}
		}
	}

	fclose(file);

	return GLUS_TRUE;
}
