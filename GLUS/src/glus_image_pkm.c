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

GLUSboolean GLUSAPIENTRY glusImageLoadPkm(const GLUSchar* filename, GLUSpkmimage* pkmimage)
{
	GLUSbinaryfile binaryfile;

	GLUSubyte* buffer;

	GLUSubyte type;

	// check, if we have a valid pointer
	if (!filename || !pkmimage)
	{
		return GLUS_FALSE;
	}

	if (!glusFileLoadBinary(filename, &binaryfile))
	{
		return GLUS_FALSE;
	}

	buffer = binaryfile.binary;
	if (!(buffer[0] == 'P' && buffer[1] == 'K' && buffer[2] == 'M' && buffer[3] == ' '))
	{
		glusFileDestroyBinary(&binaryfile);

		return GLUS_FALSE;
	}
	buffer += 7;

	pkmimage->depth = 1;

	pkmimage->imageSize = binaryfile.length - 16;
	if (pkmimage->imageSize <= 0)
	{
		glusFileDestroyBinary(&binaryfile);

		return GLUS_FALSE;
	}

	pkmimage->data = (GLUSubyte*)glusMemoryMalloc(pkmimage->imageSize * sizeof(GLUSubyte));
	if (!pkmimage->data)
	{
		glusFileDestroyBinary(&binaryfile);

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
			glusImageDestroyPkm(pkmimage);

			glusFileDestroyBinary(&binaryfile);

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

	glusFileDestroyBinary(&binaryfile);

	return GLUS_TRUE;
}

GLUSvoid GLUSAPIENTRY glusImageDestroyPkm(GLUSpkmimage* pkmimage)
{
	if (!pkmimage)
	{
		return;
	}

	if (pkmimage->data)
	{
		glusMemoryFree(pkmimage->data);

		pkmimage->data = 0;
	}

	pkmimage->width = 0;

	pkmimage->height = 0;

	pkmimage->depth = 0;

	pkmimage->internalformat = 0;

	pkmimage->imageSize = 0;
}
