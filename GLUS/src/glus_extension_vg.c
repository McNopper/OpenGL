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

GLUSboolean GLUSAPIENTRY glusExtensionIsSupported(const GLUSchar* extension)
{
	const GLUSubyte* allExtensions;
	const GLUSubyte* startExtension;
	GLUSubyte* walkerExtension;
	GLUSubyte* terminatorExtension;

	if (!extension)
	{
		return GLUS_FALSE;
	}

	walkerExtension = (GLUSubyte*)strchr(extension, ' ');
	if (walkerExtension || *extension == '\0')
	{
		return GLUS_FALSE;
	}

	allExtensions = vgGetString(VG_EXTENSIONS);

	startExtension = allExtensions;
	while (startExtension)
	{
		walkerExtension = (GLUSubyte*)strstr((const GLUSchar*)startExtension, extension);

		if (!walkerExtension)
		{
			return GLUS_FALSE;
		}

		terminatorExtension = walkerExtension + strlen(extension);

		if (!terminatorExtension)
		{
			return GLUS_FALSE;
		}

		if (walkerExtension == startExtension || *(walkerExtension - 1) == ' ')
		{
			if (*terminatorExtension == ' ' || *terminatorExtension == '\0')
			{
				return GLUS_TRUE;
			}
		}

		startExtension = (const GLUSubyte*)terminatorExtension;
	}

	return GLUS_FALSE;
}
