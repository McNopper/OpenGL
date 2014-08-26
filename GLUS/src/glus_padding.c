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

GLUSboolean GLUSAPIENTRY glusPaddingConvertf(GLUSfloat* target, const GLUSfloat* source, const GLUSint channels, const GLUSint padding, const GLUSint numberElements)
{
	GLUSint currentElement, currentChannel;

	if (!target || !source)
	{
		return GLUS_FALSE;
	}

	if (channels < 1 || padding < 0 || numberElements < 1)
	{
		return GLUS_FALSE;
	}

	for (currentElement = 0; currentElement < numberElements; currentElement++)
	{
		for (currentChannel = 0; currentChannel < channels + padding; currentChannel++)
		{
			if (currentChannel < channels)
			{
				target[currentElement * (channels + padding) + currentChannel] = source[currentElement * channels + currentChannel];
			}
			else
			{
				target[currentElement * (channels + padding) + currentChannel] = 0.0f;
			}
		}
	}

	return GLUS_TRUE;
}
