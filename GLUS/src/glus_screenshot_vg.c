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

GLUSboolean GLUSAPIENTRY glusScreenshotUseTga(GLUSint x, GLUSint y, const GLUStgaimage* screenshot)
{
	if (!screenshot)
	{
		return GLUS_FALSE;
	}

	if (x < 0 || y < 0 || screenshot->width < 1 || screenshot->height < 1 || screenshot->depth != 1 || screenshot->format != GLUS_RGBA)
	{
		return GLUS_FALSE;
	}

	vgFlush();

	vgReadPixels(screenshot->data, screenshot->width * 4 * sizeof(GLUSubyte),  VG_sABGR_8888, x, y, screenshot->width, screenshot->height);

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusScreenshotCreateTga(GLUSint x, GLUSint y, GLUSsizei width, GLUSsizei height, GLUStgaimage* screenshot)
{
	if (!screenshot)
	{
		return GLUS_FALSE;
	}

	screenshot->data = (GLUSubyte*)glusMemoryMalloc(width * height * 4);
	if (!screenshot->data)
	{
		return GLUS_FALSE;
	}
	screenshot->format= GLUS_RGBA;
	screenshot->width = width;
	screenshot->height = height;
	screenshot->depth = 1;

	return glusScreenshotUseTga(x, y, screenshot);
}
