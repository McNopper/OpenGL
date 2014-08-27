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

#define GLUS_MAX_DIMENSION 16384

extern GLUSvoid _glusImageGatherSamplePoints(GLUSint sampleIndex[4], GLUSfloat sampleWeight[2], const GLUSfloat st[2], GLUSint width, GLUSint height, GLUSint stride);

extern GLUSboolean _glusFileCheckRead(FILE* f, size_t actualRead, size_t expectedRead);
extern GLUSboolean _glusFileCheckWrite(FILE* f, size_t actualWrite, size_t expectedWrite);

static GLUSvoid glusImageSwapColorChannel(GLUSint width, GLUSint height, GLUSenum format, GLUSubyte* data)
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

GLUSboolean GLUSAPIENTRY glusImageCreateTga(GLUStgaimage* tgaimage, GLUSint width, GLUSint height, GLUSint depth, GLUSenum format)
{
	GLUSint stride;

	if (!tgaimage || width < 1 || height < 1 || depth < 1)
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

	tgaimage->data = (GLUSubyte*)glusMemoryMalloc(width * height * depth * stride * sizeof(GLUSubyte));
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

GLUSboolean GLUSAPIENTRY glusImageLoadTga(const GLUSchar* filename, GLUStgaimage* tgaimage)
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

	if (!_glusFileCheckRead(file, elementsRead, 1))
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

		if (!_glusFileCheckRead(file, elementsRead, 1))
		{
			glusImageDestroyTga(tgaimage);

			return GLUS_FALSE;
		}

		elementsRead = fread(&colorMapLength, 2, 1, file);

		if (!_glusFileCheckRead(file, elementsRead, 1))
		{
			glusImageDestroyTga(tgaimage);

			return GLUS_FALSE;
		}

		elementsRead = fread(&colorMapEntrySize, 1, 1, file);

		if (!_glusFileCheckRead(file, elementsRead, 1))
		{
			glusImageDestroyTga(tgaimage);

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

	if (!_glusFileCheckRead(file, elementsRead, 1))
	{
		glusImageDestroyTga(tgaimage);

		return GLUS_FALSE;
	}

	if (tgaimage->width > GLUS_MAX_DIMENSION)
	{
		glusImageDestroyTga(tgaimage);

		return GLUS_FALSE;
	}

	// read the height
	elementsRead = fread(&tgaimage->height, 2, 1, file);

	if (!_glusFileCheckRead(file, elementsRead, 1))
	{
		glusImageDestroyTga(tgaimage);

		return GLUS_FALSE;
	}

	if (tgaimage->height > GLUS_MAX_DIMENSION)
	{
		glusImageDestroyTga(tgaimage);

		return GLUS_FALSE;
	}

	tgaimage->depth = 1;

	// read the bits per pixel
	elementsRead = fread(&bitsPerPixel, 1, 1, file);

	if (!_glusFileCheckRead(file, elementsRead, 1))
	{
		glusImageDestroyTga(tgaimage);

		return GLUS_FALSE;
	}

	// check the pixel depth
	if (bitsPerPixel != 8 && bitsPerPixel != 24 && bitsPerPixel != 32)
	{
		fclose(file);

		glusImageDestroyTga(tgaimage);

		return GLUS_FALSE;
	}
	else
	{
		tgaimage->format = GLUS_SINGLE_CHANNEL;
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

		glusImageDestroyTga(tgaimage);

		return GLUS_FALSE;
	}

	if (hasColorMap)
	{
		// Create color map space.

		GLUSint bytesPerPixel = colorMapEntrySize / 8;

		colorMap = (GLUSubyte*)glusMemoryMalloc((size_t)colorMapLength * bytesPerPixel * sizeof(GLUSubyte));

		if (!colorMap)
		{
			fclose(file);

			glusImageDestroyTga(tgaimage);

			return GLUS_FALSE;
		}

		// Read in the color map.

		elementsRead = fread(colorMap, 1, (size_t)colorMapLength * bytesPerPixel * sizeof(GLUSubyte), file);

		if (!_glusFileCheckRead(file, elementsRead, (size_t)colorMapLength * bytesPerPixel * sizeof(GLUSubyte)))
		{
			glusImageDestroyTga(tgaimage);

			glusMemoryFree(colorMap);
			colorMap = 0;

			return GLUS_FALSE;
		}

		// swap the color if necessary
		if (colorMapEntrySize == 24 || colorMapEntrySize == 32)
		{
			glusImageSwapColorChannel(colorMapLength, 1, colorMapEntrySize == 24 ? GLUS_RGB : GLUS_RGBA, colorMap);
		}
	}

	// allocate enough memory for the targa  data
	tgaimage->data = (GLUSubyte*)glusMemoryMalloc((size_t)tgaimage->width * tgaimage->height * bitsPerPixel / 8);

	// verify memory allocation
	if (!tgaimage->data)
	{
		fclose(file);

		glusImageDestroyTga(tgaimage);

		if (hasColorMap)
		{
			glusMemoryFree(colorMap);
			colorMap = 0;
		}

		return GLUS_FALSE;
	}

	if (imageType == 1 || imageType == 2 || imageType == 3)
	{
		// read in the raw data
		elementsRead = fread(tgaimage->data, 1, (size_t)tgaimage->width * tgaimage->height * bitsPerPixel / 8, file);

		if (!_glusFileCheckRead(file, elementsRead, (size_t)tgaimage->width * tgaimage->height * bitsPerPixel / 8))
		{
			glusImageDestroyTga(tgaimage);

			if (hasColorMap)
			{
				glusMemoryFree(colorMap);
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

			if (!_glusFileCheckRead(file, elementsRead, 1))
			{
				glusImageDestroyTga(tgaimage);

				if (hasColorMap)
				{
					glusMemoryFree(colorMap);
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

				if (!_glusFileCheckRead(file, elementsRead, bitsPerPixel / 8))
				{
					glusImageDestroyTga(tgaimage);

					if (hasColorMap)
					{
						glusMemoryFree(colorMap);
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

				if (!_glusFileCheckRead(file, elementsRead, (size_t)amount * bitsPerPixel / 8))
				{
					glusImageDestroyTga(tgaimage);

					if (hasColorMap)
					{
						glusMemoryFree(colorMap);
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
		glusImageSwapColorChannel(tgaimage->width, tgaimage->height, tgaimage->format, tgaimage->data);
	}

	// close the file
	fclose(file);

	if (hasColorMap)
	{
		GLUSubyte* data = tgaimage->data;

		GLUSint bytesPerPixel;

		// Allocating new memory, as current memory is a look up table index and not a color.

		bytesPerPixel = colorMapEntrySize / 8;

		tgaimage->data = (GLUSubyte*)glusMemoryMalloc((size_t)tgaimage->width * tgaimage->height * bytesPerPixel);

		if (!tgaimage->data)
		{
			glusImageDestroyTga(tgaimage);

			glusMemoryFree(data);
			data = 0;

			glusMemoryFree(colorMap);
			colorMap = 0;

			return GLUS_FALSE;
		}

		tgaimage->format = GLUS_SINGLE_CHANNEL;
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

		glusMemoryFree(data);
		data = 0;

		glusMemoryFree(colorMap);
		colorMap = 0;
	}

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusImageSaveTga(const GLUSchar* filename, const GLUStgaimage* tgaimage)
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
		case GLUS_ALPHA:
		case GLUS_RED:
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

	if (!_glusFileCheckWrite(file, elementsWritten, 12))
	{
		return GLUS_FALSE;
	}

	elementsWritten = fwrite(&tgaimage->width, 1, sizeof(tgaimage->width), file);

	if (!_glusFileCheckWrite(file, elementsWritten, 2))
	{
		return GLUS_FALSE;
	}

	elementsWritten = fwrite(&tgaimage->height, 1, sizeof(tgaimage->height), file);

	if (!_glusFileCheckWrite(file, elementsWritten, 2))
	{
		return GLUS_FALSE;
	}

	elementsWritten = fwrite(&bitsPerPixel, 1, sizeof(bitsPerPixel), file);

	if (!_glusFileCheckWrite(file, elementsWritten, 1))
	{
		return GLUS_FALSE;
	}

	buffer[0] = 0;

	elementsWritten = fwrite(buffer, 1, 1, file);

	if (!_glusFileCheckWrite(file, elementsWritten, 1))
	{
		return GLUS_FALSE;
	}

	data = glusMemoryMalloc(tgaimage->width * tgaimage->height * bitsPerPixel / 8);

	if (!data)
	{
		fclose(file);

		return GLUS_FALSE;
	}

	memcpy(data, tgaimage->data, tgaimage->width * tgaimage->height * bitsPerPixel / 8);

	if (bitsPerPixel >= 24)
	{
		glusImageSwapColorChannel(tgaimage->width, tgaimage->height, tgaimage->format, data);
	}

	elementsWritten = fwrite(data, 1, tgaimage->width * tgaimage->height * bitsPerPixel / 8, file);

	glusMemoryFree(data);

	if (!_glusFileCheckWrite(file, elementsWritten, tgaimage->width * tgaimage->height * bitsPerPixel / 8))
	{
		return GLUS_FALSE;
	}

	fclose(file);

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusImageDestroyTga(GLUStgaimage* tgaimage)
{
	if (!tgaimage)
	{
		return;
	}

	if (tgaimage->data)
	{
		glusMemoryFree(tgaimage->data);

		tgaimage->data = 0;
	}

	tgaimage->width = 0;

	tgaimage->height = 0;

	tgaimage->depth = 0;

	tgaimage->format = 0;
}

GLUSboolean GLUSAPIENTRY glusImageSampleTga2D(GLUSubyte rgba[4], const GLUStgaimage* tgaimage, const GLUSfloat st[2])
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

	_glusImageGatherSamplePoints(sampleIndex, sampleWeight, st, tgaimage->width, tgaimage->height, stride);

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
