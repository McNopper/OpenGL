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

extern GLUSboolean glusCheckFileRead(FILE* f, size_t actualRead, size_t expectedRead);
extern GLUSboolean glusCheckFileWrite(FILE* f, size_t actualWrite, size_t expectedWrite);

GLUSboolean GLUSAPIENTRY glusFileLoadText(const GLUSchar* filename, GLUStextfile* textfile)
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

	textfile->text = (GLUSchar*)glusMemoryMalloc((size_t)textfile->length + 1);

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
		glusFileDestroyText(textfile);

		return GLUS_FALSE;
	}

	fclose(f);

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusFileSaveText(const GLUSchar* filename, const GLUStextfile* textfile)
{
	FILE* file;
	size_t elementsWritten;

	if (!filename || !textfile)
	{
		return GLUS_FALSE;
	}

	file = fopen(filename, "w");

	if (!file)
	{
		return GLUS_FALSE;
	}

	elementsWritten = fwrite(textfile->text, 1, textfile->length * sizeof(GLUSchar), file);

	if (!glusCheckFileWrite(file, elementsWritten, textfile->length * sizeof(GLUSchar)))
	{
		return GLUS_FALSE;
	}

	fclose(file);

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusFileDestroyText(GLUStextfile* textfile)
{
	if (!textfile)
	{
		return;
	}

	if (textfile->text)
	{
		glusMemoryFree(textfile->text);

		textfile->text = 0;
	}

	textfile->length = 0;
}
