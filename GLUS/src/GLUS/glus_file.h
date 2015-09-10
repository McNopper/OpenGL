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

#ifndef GLUS_FILE_H_
#define GLUS_FILE_H_

/**
 * Structure used for text file loading.
 */
typedef struct _GLUStextfile
{
	/**
	 * Contains the data of the text file.
	 */
    GLUSchar* text;

    /**
     * The length of the text file without the null terminator.
     */
    GLUSint length;

} GLUStextfile;

/**
 * Structure used for binary file loading.
 */
typedef struct _GLUSbinaryfile
{
    /**
     * The binary data of the file.
     */
    GLUSubyte* binary;

    /**
     * The length of the binary data.
     */
    GLUSint length;

} GLUSbinaryfile;

/**
 * Opens the file whose name is specified in the parameter filename and
 * associates it with a stream that can be identified in future operations by the FILE pointer returned.
 *
 * @param filename C string containing the name of the file to be opened.
 * @param mode C string containing a file access mode
 *
 * @return If the file is successfully opened, the function returns a pointer to a FILE object that can be used to identify the stream on future operations.
 *		   Otherwise, a null pointer is returned.
 */
GLUSAPI FILE* GLUSAPIENTRY glusFileOpen(const char * filename, const char * mode);


/**
 * Closes the file associated with the stream and disassociates it.
 *
 * @param stream Pointer to a FILE object that specifies the stream to be closed.
 *
 * @return If the stream is successfully closed, a zero value is returned.
 * On failure, EOF is returned.
 */
GLUSAPI int GLUSAPIENTRY glusFileClose(FILE* stream);

#endif /* GLUS_FILE_H_ */
