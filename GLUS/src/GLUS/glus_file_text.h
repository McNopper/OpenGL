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

#ifndef GLUS_FILE_TEXT_H_
#define GLUS_FILE_TEXT_H_

/**
 * Loads a text file.
 *
 * @param filename The name of the file to load.
 * @param textfile The structure to fill the text data.
 *
 * @return GLUS_TRUE, if loading succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusFileLoadText(const GLUSchar* filename, GLUStextfile* textfile);

/**
 * Saves a text file.
 *
 * @param filename	The name of the file to save.
 * @param textfile 	The structure with the text data.
 *
 * @return GLUS_TRUE, if saving succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusFileSaveText(const GLUSchar* filename, const GLUStextfile* textfile);

/**
 * Destroys the content of a text structure. Has to be called for freeing the resources.
 *
 * @param textfile The text file structure.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusFileDestroyText(GLUStextfile* textfile);

#endif /* GLUS_FILE_TEXT_H_ */
