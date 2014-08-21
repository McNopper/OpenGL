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

#define GLUS_MAX_BINARYILE_LENGTH 2147483647

extern GLUSboolean glusCheckFileRead(FILE* f, size_t actualRead, size_t expectedRead);
extern GLUSboolean glusCheckFileWrite(FILE* f, size_t actualWrite, size_t expectedWrite);

GLUSboolean GLUSAPIENTRY glusFileLoadBinary(const GLUSchar* filename, GLUSbinaryfile* binaryfile)
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

	binaryfile->binary = (GLUSubyte*)glusMemoryMalloc((size_t)binaryfile->length);

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
		glusFileDestroyBinary(binaryfile);

		return GLUS_FALSE;
	}

	fclose(f);

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusFileSaveBinary(const GLUSchar* filename, const GLUSbinaryfile* binaryfile)
{
	FILE* file;
	size_t elementsWritten;

	if (!filename || !binaryfile)
	{
		return GLUS_FALSE;
	}

	file = fopen(filename, "wb");

	if (!file)
	{
		return GLUS_FALSE;
	}

	elementsWritten = fwrite(binaryfile->binary, 1, binaryfile->length * sizeof(GLUSubyte), file);

	if (!glusCheckFileWrite(file, elementsWritten, binaryfile->length * sizeof(GLUSubyte)))
	{
		return GLUS_FALSE;
	}

	fclose(file);

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusFileDestroyBinary(GLUSbinaryfile* binaryfile)
{
	if (!binaryfile)
	{
		return;
	}

	if (binaryfile->binary)
	{
		glusMemoryFree(binaryfile->binary);

		binaryfile->binary = 0;
	}

	binaryfile->length = 0;
}
