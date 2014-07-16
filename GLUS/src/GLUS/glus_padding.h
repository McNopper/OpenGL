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

#ifndef GLUS_PADDING_H_
#define GLUS_PADDING_H_

/**
 * Converts the padding of an array of vectors.
 *
 * @param target			Target array.
 * @param source			Source array.
 * @param channels			Number of channels. Values equal greater 1 are allowed.
 * @param padding			Number of padding elements. Values equal greater 0 are allowed.
 * @param numberElements	Number of elements in the array.
 *
 * @return GLUS_TRUE, id uscceeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusPaddingConvertf(GLUSfloat* target, const GLUSfloat* source, const GLUSint channels, const GLUSint padding, const GLUSint numberElements);

#endif /* GLUS_PADDING_H_ */
