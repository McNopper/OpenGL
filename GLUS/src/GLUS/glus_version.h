/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) 2010 - 2014 Norbert Nopper
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

#ifndef GLUS_VERSION_H_
#define GLUS_VERSION_H_

/**
 * Checks, if the given OpenGL version is supported. The function reads out and parses the version string.
 *
 * @return GLUS_TRUE, if the given version is supported.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusIsSupported(const GLUSint major, const GLUSint minor);

#endif /* GLUS_VERSION_H_ */
