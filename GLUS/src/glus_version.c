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

GLUSboolean GLUSAPIENTRY glusVersionIsSupported(const GLUSint major, const GLUSint minor)
{
    GLUSint driverMajor;
    GLUSint driverMinor;

    const GLUSchar* version = (const GLUSchar*) glGetString(GL_VERSION);

    if (!version)
    {
        return GLUS_FALSE;
    }

    if (major < 1 || minor < 0)
    {
    	return GLUS_FALSE;
    }

    driverMajor = atoi(version);

    version = strchr(version, '.');

    if (!version)
    {
        return GLUS_FALSE;
    }

    version++;

    driverMinor = atoi(version);

    if (driverMajor < major)
    {
        return GLUS_FALSE;
    }
    else if (driverMajor == major)
    {
        return driverMinor >= minor;
    }

    return GLUS_TRUE;
}
