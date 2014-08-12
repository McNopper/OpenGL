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

#define GLUS_MAX_TEXTFILE_LENGTH 2147483646
#define GLUS_MAX_BINARYILE_LENGTH 2147483647
#define GLUS_MAX_DIMENSION 16384

static GLUSboolean glusCheckFileRead(FILE* f, size_t actualRead, size_t expectedRead)
{
	if (!f)
	{
		return GLUS_FALSE;
	}

	if (actualRead < expectedRead)
	{
		fclose(f);

		return GLUS_FALSE;
	}

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusLoadTextFile(const GLUSchar* filename, GLUStextfile* textfile)
{
	FILE* f;
	size_t elementsRead;

	if (!filename || !textfile)
	{
		return GLUS_FALSE;
	}

	textfile->text = 0;

	textfile->length = 0;

	f = fopen(filename, "rb");

	if (!f)
	{
		return GLUS_FALSE;
	}

	if (fseek(f, 0, SEEK_END))
	{
		fclose(f);

		return GLUS_FALSE;
	}

	textfile->length = ftell(f);

	if (textfile->length < 0 || textfile->length == GLUS_MAX_TEXTFILE_LENGTH)
	{
		fclose(f);

		textfile->length = 0;

		return GLUS_FALSE;
	}

	textfile->text = (GLUSchar*)glusMalloc((size_t)textfile->length + 1);

	if (!textfile->text)
	{
		fclose(f);

		textfile->length = 0;

		return GLUS_FALSE;
	}

	memset(textfile->text, 0, (size_t)textfile->length + 1);

	rewind(f);

	elementsRead = fread(textfile->text, 1, (size_t)textfile->length, f);

	if (!glusCheckFileRead(f, elementsRead, (size_t)textfile->length))
	{
		glusDestroyTextFile(textfile);

		return GLUS_FALSE;
	}

	fclose(f);

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusDestroyTextFile(GLUStextfile* textfile)
{
	if (!textfile)
	{
		return;
	}

	if (textfile->text)
	{
		glusFree(textfile->text);

		textfile->text = 0;
	}

	textfile->length = 0;
}

GLUSboolean GLUSAPIENTRY glusLoadBinaryFile(const GLUSchar* filename, GLUSbinaryfile* binaryfile)
{
	FILE* f;
	size_t elementsRead;

	if (!filename || !binaryfile)
	{
		return GLUS_FALSE;
	}

	binaryfile->binary = 0;

	binaryfile->length = 0;

	f = fopen(filename, "rb");

	if (!f)
	{
		return GLUS_FALSE;
	}

	if (fseek(f, 0, SEEK_END))
	{
		fclose(f);

		return GLUS_FALSE;
	}

	binaryfile->length = ftell(f);

	if (binaryfile->length < 0 || binaryfile->length == GLUS_MAX_BINARYILE_LENGTH)
	{
		fclose(f);

		binaryfile->length = 0;

		return GLUS_FALSE;
	}

	binaryfile->binary = (GLUSubyte*)glusMalloc((size_t)binaryfile->length);

	if (!binaryfile->binary)
	{
		fclose(f);

		binaryfile->length = 0;

		return GLUS_FALSE;
	}

	memset(binaryfile->binary, 0, (size_t)binaryfile->length);

	rewind(f);

	elementsRead = fread(binaryfile->binary, 1, (size_t)binaryfile->length, f);

	if (!glusCheckFileRead(f, elementsRead, (size_t)binaryfile->length))
	{
		glusDestroyBinaryFile(binaryfile);

		return GLUS_FALSE;
	}

	fclose(f);

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusDestroyBinaryFile(GLUSbinaryfile* binaryfile)
{
	if (!binaryfile)
	{
		return;
	}

	if (binaryfile->binary)
	{
		glusFree(binaryfile->binary);

		binaryfile->binary = 0;
	}

	binaryfile->length = 0;
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

GLUSboolean GLUSAPIENTRY glusLoadTgaImage(const GLUSchar* filename, GLUStgaimage* tgaimage)
{
	FILE* file;

	GLUSboolean hasColorMap = GLUS_FALSE;

	GLUSubyte imageType;
	GLUSubyte bitsPerPixel;

	GLUSushort firstEntryIndex;
	GLUSushort colorMapLength;
	GLUSubyte colorMapEntrySize;
	GLUSubyte* colorMap = 0;

	GLUSuint i, k;

	size_t elementsRead;

	// check, if we have a valid pointer
	if (!filename || !tgaimage)
	{
		return GLUS_FALSE;
	}

	tgaimage->width = 0;
	tgaimage->height = 0;
	tgaimage->depth = 0;
	tgaimage->data = 0;
	tgaimage->format = 0;

	// open filename in "read binary" mode
	file = fopen(filename, "rb");

	if (!file)
	{
		return GLUS_FALSE;
	}

	// seek through the tga header, up to the type:
	if (fseek(file, 2, SEEK_CUR))
	{
		fclose(file);

		return GLUS_FALSE;
	}

	// read the image type
	elementsRead = fread(&imageType, 1, 1, file);

	if (!glusCheckFileRead(file, elementsRead, 1))
	{
		return GLUS_FALSE;
	}

	// check the type
	if (imageType != 1 && imageType != 2 && imageType != 3 && imageType != 9 && imageType != 10 && imageType != 11)
	{
		fclose(file);

		return GLUS_FALSE;
	}

	if (imageType == 1 || imageType == 9)
	{
		hasColorMap = GLUS_TRUE;
	}

	if (!hasColorMap)
	{
		// seek through the tga header, up to the width/height:
		if (fseek(file, 9, SEEK_CUR))
		{
			fclose(file);

			return GLUS_FALSE;
		}
	}
	else
	{
		elementsRead = fread(&firstEntryIndex, 2, 1, file);

		if (!glusCheckFileRead(file, elementsRead, 1))
		{
			glusDestroyTgaImage(tgaimage);

			return GLUS_FALSE;
		}

		elementsRead = fread(&colorMapLength, 2, 1, file);

		if (!glusCheckFileRead(file, elementsRead, 1))
		{
			glusDestroyTgaImage(tgaimage);

			return GLUS_FALSE;
		}

		elementsRead = fread(&colorMapEntrySize, 1, 1, file);

		if (!glusCheckFileRead(file, elementsRead, 1))
		{
			glusDestroyTgaImage(tgaimage);

			return GLUS_FALSE;
		}

		// seek through the tga header, up to the width/height:
		if (fseek(file, 4, SEEK_CUR))
		{
			fclose(file);

			return GLUS_FALSE;
		}
	}

	// read the width
	elementsRead = fread(&tgaimage->width, 2, 1, file);

	if (!glusCheckFileRead(file, elementsRead, 1))
	{
		glusDestroyTgaImage(tgaimage);

		return GLUS_FALSE;
	}

	if (tgaimage->width > GLUS_MAX_DIMENSION)
	{
		glusDestroyTgaImage(tgaimage);

		return GLUS_FALSE;
	}

	// read the height
	elementsRead = fread(&tgaimage->height, 2, 1, file);

	if (!glusCheckFileRead(file, elementsRead, 1))
	{
		glusDestroyTgaImage(tgaimage);

		return GLUS_FALSE;
	}

	if (tgaimage->height > GLUS_MAX_DIMENSION)
	{
		glusDestroyTgaImage(tgaimage);

		return GLUS_FALSE;
	}

	tgaimage->depth = 1;

	// read the bits per pixel
	elementsRead = fread(&bitsPerPixel, 1, 1, file);

	if (!glusCheckFileRead(file, elementsRead, 1))
	{
		glusDestroyTgaImage(tgaimage);

		return GLUS_FALSE;
	}

	// check the pixel depth
	if (bitsPerPixel != 8 && bitsPerPixel != 24 && bitsPerPixel != 32)
	{
		fclose(file);

		glusDestroyTgaImage(tgaimage);

		return GLUS_FALSE;
	}
	else
	{
		tgaimage->format = GLUS_LUMINANCE;
		if (bitsPerPixel == 24)
		{
			tgaimage->format = GLUS_RGB;
		}
		else if (bitsPerPixel == 32)
		{
			tgaimage->format = GLUS_RGBA;
		}
	}

	// move file pointer to beginning of targa data
	if (fseek(file, 1, SEEK_CUR))
	{
		fclose(file);

		glusDestroyTgaImage(tgaimage);

		return GLUS_FALSE;
	}

	if (hasColorMap)
	{
		// Create color map space.

		GLUSint bytesPerPixel = colorMapEntrySize / 8;

		colorMap = (GLUSubyte*)glusMalloc((size_t)colorMapLength * bytesPerPixel * sizeof(GLUSubyte));

		if (!colorMap)
		{
			fclose(file);

			glusDestroyTgaImage(tgaimage);

			return GLUS_FALSE;
		}

		// Read in the color map.

		elementsRead = fread(colorMap, 1, (size_t)colorMapLength * bytesPerPixel * sizeof(GLUSubyte), file);

		if (!glusCheckFileRead(file, elementsRead, (size_t)colorMapLength * bytesPerPixel * sizeof(GLUSubyte)))
		{
			glusDestroyTgaImage(tgaimage);

			glusFree(colorMap);
			colorMap = 0;

			return GLUS_FALSE;
		}

		// swap the color if necessary
		if (colorMapEntrySize == 24 || colorMapEntrySize == 32)
		{
			glusSwapColorChannel(colorMapLength, 1, colorMapEntrySize == 24 ? GLUS_RGB : GLUS_RGBA, colorMap);
		}
	}

	// allocate enough memory for the targa  data
	tgaimage->data = (GLUSubyte*)glusMalloc((size_t)tgaimage->width * tgaimage->height * bitsPerPixel / 8);

	// verify memory allocation
	if (!tgaimage->data)
	{
		fclose(file);

		glusDestroyTgaImage(tgaimage);

		if (hasColorMap)
		{
			glusFree(colorMap);
			colorMap = 0;
		}

		return GLUS_FALSE;
	}

	if (imageType == 1 || imageType == 2 || imageType == 3)
	{
		// read in the raw data
		elementsRead = fread(tgaimage->data, 1, (size_t)tgaimage->width * tgaimage->height * bitsPerPixel / 8, file);

		if (!glusCheckFileRead(file, elementsRead, (size_t)tgaimage->width * tgaimage->height * bitsPerPixel / 8))
		{
			glusDestroyTgaImage(tgaimage);

			if (hasColorMap)
			{
				glusFree(colorMap);
				colorMap = 0;
			}

			return GLUS_FALSE;
		}
	}
	else
	{
		// RLE encoded
		GLUSint pixelsRead = 0;

		while (pixelsRead < tgaimage->width * tgaimage->height)
		{
			GLUSubyte amount;

			elementsRead = fread(&amount, 1, 1, file);

			if (!glusCheckFileRead(file, elementsRead, 1))
			{
				glusDestroyTgaImage(tgaimage);

				if (hasColorMap)
				{
					glusFree(colorMap);
					colorMap = 0;
				}

				return GLUS_FALSE;
			}

			if (amount & 0x80)
			{
				GLUSint i;
				GLUSint k;

				amount &= 0x7F;

				amount++;

				// read in the rle data
				elementsRead = fread(&tgaimage->data[pixelsRead * bitsPerPixel / 8], 1, bitsPerPixel / 8, file);

				if (!glusCheckFileRead(file, elementsRead, bitsPerPixel / 8))
				{
					glusDestroyTgaImage(tgaimage);

					if (hasColorMap)
					{
						glusFree(colorMap);
						colorMap = 0;
					}

					return GLUS_FALSE;
				}

				for (i = 1; i < amount; i++)
				{
					for (k = 0; k < bitsPerPixel / 8; k++)
					{
						tgaimage->data[(pixelsRead + i) * bitsPerPixel / 8 + k] = tgaimage->data[pixelsRead * bitsPerPixel / 8 + k];
					}
				}
			}
			else
			{
				amount &= 0x7F;

				amount++;

				// read in the raw data
				elementsRead = fread(&tgaimage->data[pixelsRead * bitsPerPixel / 8], 1, (size_t)amount * bitsPerPixel / 8, file);

				if (!glusCheckFileRead(file, elementsRead, (size_t)amount * bitsPerPixel / 8))
				{
					glusDestroyTgaImage(tgaimage);

					if (hasColorMap)
					{
						glusFree(colorMap);
						colorMap = 0;
					}

					return GLUS_FALSE;
				}
			}

			pixelsRead += amount;
		}
	}

	// swap the color if necessary
	if (bitsPerPixel == 24 || bitsPerPixel == 32)
	{
		glusSwapColorChannel(tgaimage->width, tgaimage->height, tgaimage->format, tgaimage->data);
	}

	// close the file
	fclose(file);

	if (hasColorMap)
	{
		GLUSubyte* data = tgaimage->data;

		GLUSint bytesPerPixel;

		// Allocating new memory, as current memory is a look up table index and not a color.

		bytesPerPixel = colorMapEntrySize / 8;

		tgaimage->data = (GLUSubyte*)glusMalloc((size_t)tgaimage->width * tgaimage->height * bytesPerPixel);

		if (!tgaimage->data)
		{
			glusDestroyTgaImage(tgaimage);

			glusFree(data);
			data = 0;

			glusFree(colorMap);
			colorMap = 0;

			return GLUS_FALSE;
		}

		tgaimage->format = GLUS_LUMINANCE;
		if (colorMapEntrySize == 24)
		{
			tgaimage->format = GLUS_RGB;
		}
		else if (colorMapEntrySize == 32)
		{
			tgaimage->format = GLUS_RGBA;
		}

		// Copy color values from the color map into the image data.

		for (i = 0; i < (GLUSuint)tgaimage->width * (GLUSuint)tgaimage->height; i++)
		{
			for (k = 0; k < (GLUSuint)bytesPerPixel; k++)
			{
				tgaimage->data[i * bytesPerPixel + k] = colorMap[(firstEntryIndex + data[i]) * bytesPerPixel + k];
			}
		}

		// Freeing data.

		glusFree(data);
		data = 0;

		glusFree(colorMap);
		colorMap = 0;
	}

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusDestroyTgaImage(GLUStgaimage* tgaimage)
{
	if (!tgaimage)
	{
		return;
	}

	if (tgaimage->data)
	{
		glusFree(tgaimage->data);

		tgaimage->data = 0;
	}

	tgaimage->width = 0;

	tgaimage->height = 0;

	tgaimage->depth = 0;

	tgaimage->format = 0;
}

static GLUSvoid convertRGBE(GLUSfloat* rgb, const GLUSubyte* rgbe)
{
	GLUSfloat exponent = (GLUSfloat)(rgbe[3] - 128);

	rgb[0] = (GLUSfloat)rgbe[0] / 256.0f * powf(2.0f, exponent);
	rgb[1] = (GLUSfloat)rgbe[1] / 256.0f * powf(2.0f, exponent);
	rgb[2] = (GLUSfloat)rgbe[2] / 256.0f * powf(2.0f, exponent);
}

static GLUSint glusDecodeNewRLE(FILE* file, GLUSubyte* scanline, GLUSint width)
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

			if (!glusCheckFileRead(file, elementsRead, 1))
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

				if (!glusCheckFileRead(file, elementsRead, 1))
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

					if (!glusCheckFileRead(file, elementsRead, 1))
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

//
// HDR Image loading
//
// see http://radsite.lbl.gov/radiance/refer/filefmts.pdf
// see http://radsite.lbl.gov/radiance/refer/Notes/picture_format.html
// see http://radiance-online.org/cgi-bin/viewcvs.cgi/ray/src/common/color.c?view=markup
// see http://www.flipcode.com/archives/HDR_Image_Reader.shtml

GLUSboolean GLUSAPIENTRY glusLoadHdrImage(const GLUSchar* filename, GLUShdrimage* hdrimage)
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

	if (!glusCheckFileRead(file, elementsRead, 1))
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

		if (!glusCheckFileRead(file, elementsRead, 1))
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

		if (!glusCheckFileRead(file, elementsRead, 1))
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

		glusDestroyHdrImage(hdrimage);

		return GLUS_FALSE;
	}

	hdrimage->width = (GLUSushort)width;
	hdrimage->height = (GLUSushort)height;
	hdrimage->depth = 1;
	hdrimage->format = GLUS_RGB;

	hdrimage->data = (GLUSfloat*)glusMalloc(width * height * 3 * sizeof(GLUSfloat));

	if (!hdrimage->data)
	{
		fclose(file);

		glusDestroyHdrImage(hdrimage);

		return GLUS_FALSE;
	}

	// Scanlines
	scanline = (GLUSubyte*)glusMalloc(width * 4 * sizeof(GLUSubyte));

	if (!scanline)
	{
		glusFree(scanline);

		fclose(file);

		glusDestroyHdrImage(hdrimage);

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

		if (!glusCheckFileRead(file, elementsRead, 1))
		{
			glusFree(scanline);

			glusDestroyHdrImage(hdrimage);

			return GLUS_FALSE;
		}

		repeat = 0;

		// Examine value
		if (width < 32768 && buffer[0] == 2 && buffer[1] == 2 && buffer[2] == ((width >> 8) & 0xFF) && buffer[3] == (width & 0xFF))
		{
			// New RLE decoding

			GLUSint scanlinePixels = glusDecodeNewRLE(file, scanline, width);

			if (scanlinePixels < 0)
			{
				glusFree(scanline);

				// File already closed

				glusDestroyHdrImage(hdrimage);

				return GLUS_FALSE;
			}

			for (i = 0; i < scanlinePixels; i++)
			{
				if (y < 0)
				{
					glusFree(scanline);

					fclose(file);

					glusDestroyHdrImage(hdrimage);

					return GLUS_FALSE;
				}

				convertRGBE(rgb, &scanline[i * 4]);

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

		convertRGBE(rgb, rgbe);

		while (repeat)
		{
			if (y < 0)
			{
				glusFree(scanline);

				fclose(file);

				glusDestroyHdrImage(hdrimage);

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

	glusFree(scanline);

	fclose(file);

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusDestroyHdrImage(GLUShdrimage* hdrimage)
{
	if (!hdrimage)
	{
		return;
	}

	if (hdrimage->data)
	{
		glusFree(hdrimage->data);

		hdrimage->data = 0;
	}

	hdrimage->width = 0;

	hdrimage->height = 0;

	hdrimage->depth = 0;

	hdrimage->format = 0;
}

GLUSboolean GLUSAPIENTRY glusLoadPkmImage(const GLUSchar* filename, GLUSpkmimage* pkmimage)
{
	GLUSbinaryfile binaryfile;

	GLUSubyte* buffer;

	GLUSubyte type;

	// check, if we have a valid pointer
	if (!filename || !pkmimage)
	{
		return GLUS_FALSE;
	}

	if (!glusLoadBinaryFile(filename, &binaryfile))
	{
		return GLUS_FALSE;
	}

	buffer = binaryfile.binary;
	if (!(buffer[0] == 'P' && buffer[1] == 'K' && buffer[2] == 'M' && buffer[3] == ' '))
	{
		glusDestroyBinaryFile(&binaryfile);

		return GLUS_FALSE;
	}
	buffer += 7;

	pkmimage->depth = 1;

	pkmimage->imageSize = binaryfile.length - 16;
	if (pkmimage->imageSize <= 0)
	{
		glusDestroyBinaryFile(&binaryfile);

		return GLUS_FALSE;
	}

	pkmimage->data = (GLUSubyte*)glusMalloc(pkmimage->imageSize * sizeof(GLUSubyte));
	if (!pkmimage->data)
	{
		glusDestroyBinaryFile(&binaryfile);

		return GLUS_FALSE;
	}

	type = *buffer;
	switch (type)
	{
		case 1:
			pkmimage->internalformat = GLUS_COMPRESSED_RGB8_ETC2;
		break;
		case 3:
			pkmimage->internalformat = GLUS_COMPRESSED_RGBA8_ETC2_EAC;
		break;
		case 4:
			pkmimage->internalformat = GLUS_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
		break;
		case 5:
			pkmimage->internalformat = GLUS_COMPRESSED_R11_EAC;
		break;
		case 6:
			pkmimage->internalformat = GLUS_COMPRESSED_RG11_EAC;
		break;
		case 7:
			pkmimage->internalformat = GLUS_COMPRESSED_SIGNED_R11_EAC;
		break;
		case 8:
			pkmimage->internalformat = GLUS_COMPRESSED_SIGNED_RG11_EAC;
		break;
		default:
		{
			glusDestroyPkmImage(pkmimage);

			glusDestroyBinaryFile(&binaryfile);

			return GLUS_FALSE;
		}
		break;
	}
	buffer += 5;

	pkmimage->width = (GLUSushort)(*buffer) * 256;
	buffer += 1;
	pkmimage->width += (GLUSushort)(*buffer);
	buffer += 1;

	pkmimage->height = (GLUSushort)(*buffer) * 256;
	buffer += 1;
	pkmimage->height += (GLUSushort)(*buffer);
	buffer += 1;

	memcpy(pkmimage->data, buffer, pkmimage->imageSize);

	glusDestroyBinaryFile(&binaryfile);

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusDestroyPkmImage(GLUSpkmimage* pkmimage)
{
	if (!pkmimage)
	{
		return;
	}

	if (pkmimage->data)
	{
		glusFree(pkmimage->data);

		pkmimage->data = 0;
	}

	pkmimage->width = 0;

	pkmimage->height = 0;

	pkmimage->depth = 0;

	pkmimage->internalformat = 0;

	pkmimage->imageSize = 0;
}
