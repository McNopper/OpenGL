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

extern GLUSvoid glusImageGatherSamplePoints(GLUSint sampleIndex[4], GLUSfloat sampleWeight[2], const GLUSfloat st[2], GLUSint width, GLUSint height, GLUSint stride);

extern GLUSboolean glusFileCheckRead(FILE* f, size_t actualRead, size_t expectedRead);
extern GLUSboolean glusFileCheckWrite(FILE* f, size_t actualWrite, size_t expectedWrite);

static GLUSvoid glusImageConvertRGBE(GLUSfloat* rgb, const GLUSubyte* rgbe)
{
	GLUSfloat exponent = (GLUSfloat)(rgbe[3] - 128);

	rgb[0] = (GLUSfloat)rgbe[0] / 256.0f * powf(2.0f, exponent);
	rgb[1] = (GLUSfloat)rgbe[1] / 256.0f * powf(2.0f, exponent);
	rgb[2] = (GLUSfloat)rgbe[2] / 256.0f * powf(2.0f, exponent);
}
static GLUSvoid glusImageConvertRGB(GLUSubyte* rgbe, const GLUSfloat* rgb)
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

static GLUSint glusImageDecodeNewRLE(FILE* file, GLUSubyte* scanline, GLUSint width)
{
	GLUSint channel, x, scanLength, maxScanLength;
	GLUSubyte code, channelValue;
	size_t elementsRead;

	maxScanLength = 0;

	// read each component
	for (channel = 0; channel < 4; channel++)
	{
		x = 0;

		scanLength = 0;

		while (x < width)
		{
			elementsRead = fread(&code, 1, 1, file);

			if (!glusFileCheckRead(file, elementsRead, 1))
			{
				return -1;
			}

			if (code > 128)
			{
				// Run

				code &= 127;

				scanLength += code;

				if (scanLength > width)
				{
					fclose(file);

					return -1;
				}

				elementsRead = fread(&channelValue, 1, 1, file);

				if (!glusFileCheckRead(file, elementsRead, 1))
				{
					return -1;
				}

				while (code--)
				{
					scanline[x++ * 4 + channel] = channelValue;
				}
			}
			else
			{
				// Non-run

				scanLength += code;

				if (scanLength > width)
				{
					fclose(file);

					return -1;
				}

				while (code--)
				{
					elementsRead = fread(&channelValue, 1, 1, file);

					if (!glusFileCheckRead(file, elementsRead, 1))
					{
						return -1;
					}

					scanline[x++ * 4 + channel] = channelValue;
				}
			}
		}

		if (scanLength > maxScanLength)
		{
			maxScanLength = scanLength;
		}
	}

	return maxScanLength;
}

GLUSboolean GLUSAPIENTRY glusImageCreateHdr(GLUShdrimage* hdrimage, GLUSint width, GLUSint height, GLUSint depth, GLUSenum format)
{
	GLUSint stride;

	if (!hdrimage || width < 1 || height < 1 || depth < 1)
	{
		return GLUS_FALSE;
	}

	if (format == GLUS_ALPHA || format == GLUS_LUMINANCE || format == GLUS_RED)
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

	hdrimage->data = (GLUSfloat*)glusMemoryMalloc(width * height * depth * stride * sizeof(GLUSfloat));
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

//
// HDR Image loading
//
// see http://radsite.lbl.gov/radiance/refer/filefmts.pdf
// see http://radsite.lbl.gov/radiance/refer/Notes/picture_format.html
// see http://radiance-online.org/cgi-bin/viewcvs.cgi/ray/src/common/color.c?view=markup
// see http://www.flipcode.com/archives/HDR_Image_Reader.shtml

GLUSboolean GLUSAPIENTRY glusImageLoadHdr(const GLUSchar* filename, GLUShdrimage* hdrimage)
{
	FILE* file;

	GLUSchar buffer[256];
	GLUSchar currentChar, oldChar;

	GLUSint width, height, x, y, repeat, factor, i;

	GLUSubyte* scanline;
	GLUSubyte rgbe[4];
	GLUSubyte prevRgbe[4];

	GLUSfloat rgb[3];

	size_t elementsRead;

	// check, if we have a valid pointer
	if (!filename || !hdrimage)
	{
		return GLUS_FALSE;
	}

	hdrimage->width = 0;
	hdrimage->height = 0;
	hdrimage->depth = 0;
	hdrimage->data = 0;

	// open filename in "read binary" mode
	file = fopen(filename, "rb");

	if (!file)
	{
		return GLUS_FALSE;
	}

	elementsRead = fread(buffer, 10, 1, file);

	if (!glusFileCheckRead(file, elementsRead, 1))
	{
		return GLUS_FALSE;
	}

	//
	// Information header
	//

	// Identifier
	if (strncmp(buffer, "#?RADIANCE", 10))
	{
		fclose(file);

		return GLUS_FALSE;
	}

	// Go to variables
	if (fseek(file, 1, SEEK_CUR))
	{
		fclose(file);

		return GLUS_FALSE;
	}

	// Variables
	currentChar = 0;
	while (GLUS_TRUE)
	{
		oldChar = currentChar;

		elementsRead = fread(&currentChar, 1, 1, file);

		if (!glusFileCheckRead(file, elementsRead, 1))
		{
			return GLUS_FALSE;
		}

		// Empty line indicates end of header
		if (currentChar == '\n' && oldChar == '\n')
		{
			break;
		}
	}

	// Resolution
	i = 0;
	while (GLUS_TRUE)
	{
		elementsRead = fread(&currentChar, 1, 1, file);

		if (!glusFileCheckRead(file, elementsRead, 1))
		{
			return GLUS_FALSE;
		}

		buffer[i++] = currentChar;

		if (currentChar == '\n')
		{
			break;
		}
	}

	if (!sscanf(buffer, "-Y %d +X %d", &height, &width))
	{
		fclose(file);

		glusImageDestroyHdr(hdrimage);

		return GLUS_FALSE;
	}

	hdrimage->width = (GLUSushort)width;
	hdrimage->height = (GLUSushort)height;
	hdrimage->depth = 1;
	hdrimage->format = GLUS_RGB;

	hdrimage->data = (GLUSfloat*)glusMemoryMalloc(width * height * 3 * sizeof(GLUSfloat));

	if (!hdrimage->data)
	{
		fclose(file);

		glusImageDestroyHdr(hdrimage);

		return GLUS_FALSE;
	}

	// Scanlines
	scanline = (GLUSubyte*)glusMemoryMalloc(width * 4 * sizeof(GLUSubyte));

	if (!scanline)
	{
		glusMemoryFree(scanline);

		fclose(file);

		glusImageDestroyHdr(hdrimage);

		return GLUS_FALSE;
	}

	prevRgbe[0] = 0;
	prevRgbe[1] = 0;
	prevRgbe[2] = 0;
	prevRgbe[3] = 0;

	factor = 1;
	x = 0;
	y = height - 1;
	while (y >= 0)
	{
		elementsRead = fread(buffer, 4, 1, file);

		if (!glusFileCheckRead(file, elementsRead, 1))
		{
			glusMemoryFree(scanline);

			glusImageDestroyHdr(hdrimage);

			return GLUS_FALSE;
		}

		repeat = 0;

		// Examine value
		if (width < 32768 && buffer[0] == 2 && buffer[1] == 2 && buffer[2] == ((width >> 8) & 0xFF) && buffer[3] == (width & 0xFF))
		{
			// New RLE decoding

			GLUSint scanlinePixels = glusImageDecodeNewRLE(file, scanline, width);

			if (scanlinePixels < 0)
			{
				glusMemoryFree(scanline);

				// File already closed

				glusImageDestroyHdr(hdrimage);

				return GLUS_FALSE;
			}

			for (i = 0; i < scanlinePixels; i++)
			{
				if (y < 0)
				{
					glusMemoryFree(scanline);

					fclose(file);

					glusImageDestroyHdr(hdrimage);

					return GLUS_FALSE;
				}

				glusImageConvertRGBE(rgb, &scanline[i * 4]);

				hdrimage->data[(width * y + x) * 3 + 0] = rgb[0];
				hdrimage->data[(width * y + x) * 3 + 1] = rgb[1];
				hdrimage->data[(width * y + x) * 3 + 2] = rgb[2];

				x++;
				if (x >= width)
				{
					y--;
					x = 0;
				}
			}

			factor = 1;

			prevRgbe[0] = scanline[(scanlinePixels - 1) * 4 + 0];
			prevRgbe[1] = scanline[(scanlinePixels - 1) * 4 + 1];
			prevRgbe[2] = scanline[(scanlinePixels - 1) * 4 + 2];
			prevRgbe[3] = scanline[(scanlinePixels - 1) * 4 + 3];

			continue;
		}
		else if (buffer[0] == 1 && buffer[1] == 1 && buffer[2] == 1)
		{
			// Old RLE decoding

			repeat = buffer[3] * factor;

			rgbe[0] = prevRgbe[0];
			rgbe[1] = prevRgbe[1];
			rgbe[2] = prevRgbe[2];
			rgbe[3] = prevRgbe[3];

			factor *= 256;
		}
		else
		{
			// No RLE decoding

			repeat = 1;

			rgbe[0] = buffer[0];
			rgbe[1] = buffer[1];
			rgbe[2] = buffer[2];
			rgbe[3] = buffer[3];

			factor = 1;
		}

		glusImageConvertRGBE(rgb, rgbe);

		while (repeat)
		{
			if (y < 0)
			{
				glusMemoryFree(scanline);

				fclose(file);

				glusImageDestroyHdr(hdrimage);

				return GLUS_FALSE;
			}

			hdrimage->data[(width * y + x) * 3 + 0] = rgb[0];
			hdrimage->data[(width * y + x) * 3 + 1] = rgb[1];
			hdrimage->data[(width * y + x) * 3 + 2] = rgb[2];

			x++;
			if (x >= width)
			{
				y--;
				x = 0;
			}

			repeat--;
		}

		prevRgbe[0] = rgbe[0];
		prevRgbe[1] = rgbe[1];
		prevRgbe[2] = rgbe[2];
		prevRgbe[3] = rgbe[3];
	}

	glusMemoryFree(scanline);

	fclose(file);

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusImageSaveHdr(const GLUSchar* filename, const GLUShdrimage* hdrimage)
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

	if (hdrimage->format != GLUS_RGB)
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

	if (!glusFileCheckWrite(file, elementsWritten, 52))
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
			glusImageConvertRGB(rgbe, &hdrimage->data[(y * hdrimage->width + x) * 3]);

			elementsWritten = fwrite(rgbe, 1, 4 * sizeof(GLUSubyte), file);

			if (!glusFileCheckWrite(file, elementsWritten, 4 * sizeof(GLUSubyte)))
			{
				return GLUS_FALSE;
			}
		}
	}

	fclose(file);

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusImageDestroyHdr(GLUShdrimage* hdrimage)
{
	if (!hdrimage)
	{
		return;
	}

	if (hdrimage->data)
	{
		glusMemoryFree(hdrimage->data);

		hdrimage->data = 0;
	}

	hdrimage->width = 0;

	hdrimage->height = 0;

	hdrimage->depth = 0;

	hdrimage->format = 0;
}

GLUSboolean GLUSAPIENTRY glusImageSampleHdr2D(GLUSfloat rgb[3], const GLUShdrimage* hdrimage, const GLUSfloat st[2])
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

	glusImageGatherSamplePoints(sampleIndex, sampelWeight, st, hdrimage->width, hdrimage->height, stride);

	for (i = 0; i < stride; i++)
	{
		rgb[i] = hdrimage->data[sampleIndex[0] + i] * sampelWeight[0] * sampelWeight[1];
		rgb[i] += hdrimage->data[sampleIndex[1] + i] * (1.0f - sampelWeight[0]) * sampelWeight[1];
		rgb[i] += hdrimage->data[sampleIndex[2] + i] * sampelWeight[0] * (1.0f - sampelWeight[1]);
		rgb[i] += hdrimage->data[sampleIndex[3] + i] * (1.0f - sampelWeight[0]) * (1.0f - sampelWeight[1]);
	}

	return GLUS_TRUE;
}
