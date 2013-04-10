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

GLUSboolean GLUSAPIENTRY glusScreenshot(GLUSint x, GLUSint y, GLUSsizei width, GLUSsizei height, GLUStgaimage* screenshot)
{
	if (!screenshot)
	{
		return GLUS_FALSE;
	}

	screenshot->data = (GLUSubyte*)malloc(width * height * 4);
	if (!screenshot->data)
	{
		return GLUS_FALSE;
	}
	screenshot->format= GLUS_RGBA;
	screenshot->width = width;
	screenshot->height = height;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glFlush();

	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glReadPixels(x, y, width, height, GL_RGBA, GL_UNSIGNED_BYTE, screenshot->data);

	return GLUS_TRUE;
}
