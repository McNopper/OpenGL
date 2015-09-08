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

FILE* GLUSAPIENTRY glusFileOpen(const char * filename, const char * mode)
{
	char buffer[GLUS_MAX_FILENAME];

	if (!filename)
	{
		return 0;
	}

	if (strlen(filename) + strlen(GLUS_BASE_DIRECTORY) >= GLUS_MAX_FILENAME)
	{
		return 0;
	}

	strcpy(buffer, GLUS_BASE_DIRECTORY);
	strcat(buffer, filename);

	return fopen(buffer, mode);
}

GLUSboolean _glusFileCheckRead(FILE* f, size_t actualRead, size_t expectedRead)
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

GLUSboolean _glusFileCheckWrite(FILE* f, size_t actualWrite, size_t expectedWrite)
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
