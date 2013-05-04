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
        if (ferror(f))
        {
            fclose(f);

            return GLUS_FALSE;
        }
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

    f = fopen(filename, "r");

    if (!f)
    {
        return GLUS_FALSE;
    }

    if(fseek(f, 0, SEEK_END))
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
	
    textfile->text = (GLUSchar*) malloc((size_t)textfile->length + 1);

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
        free(textfile->text);

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

    if(fseek(f, 0, SEEK_END))
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
	
    binaryfile->binary = (GLUSubyte*) malloc((size_t)binaryfile->length);

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
        free(binaryfile->binary);

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
    GLUSubyte type;
    GLUSubyte bitsPerPixel;
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
    if(fseek(file, 2, SEEK_CUR))
	{
        fclose(file);

        return GLUS_FALSE;
	}

    // read the type
    elementsRead = fread(&type, 1, 1, file);

    if (!glusCheckFileRead(file, elementsRead, 1))
    {
        return GLUS_FALSE;
    }
		
    // check the type
    if (type != 2 && type != 10 && type != 11)
    {
        fclose(file);

        return GLUS_FALSE;
    }

    // seek through the tga header, up to the width/height:
    if(fseek(file, 9, SEEK_CUR))
	{
        fclose(file);

        return GLUS_FALSE;
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
    if(fseek(file, 1, SEEK_CUR))
	{
		fclose(file);
	
        glusDestroyTgaImage(tgaimage);

        return GLUS_FALSE;
	}

    // allocate enough memory for the targa  data
    tgaimage->data = (GLUSubyte*) malloc((size_t)tgaimage->width * tgaimage->height * bitsPerPixel / 8);

    // verify memory allocation
    if (!tgaimage->data)
    {
        fclose(file);
		
		glusDestroyTgaImage(tgaimage);
		
        return GLUS_FALSE;
    }

    if (type == 2)
    {
        // read in the raw data
        elementsRead = fread(tgaimage->data, 1, (size_t)tgaimage->width * tgaimage->height * bitsPerPixel / 8, file);

        if (!glusCheckFileRead(file, elementsRead, (size_t)tgaimage->width * tgaimage->height * bitsPerPixel / 8))
        {
            glusDestroyTgaImage(tgaimage);

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
                elementsRead = fread(&tgaimage->data[pixelsRead * bitsPerPixel / 8], 1, (size_t) amount * bitsPerPixel / 8, file);
				
                if (!glusCheckFileRead(file, elementsRead, (size_t) amount * bitsPerPixel / 8))
                {
                    glusDestroyTgaImage(tgaimage);

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
        free(tgaimage->data);

        tgaimage->data = 0;
    }

    tgaimage->width = 0;

    tgaimage->height = 0;

    tgaimage->depth = 0;

    tgaimage->format = 0;
}
