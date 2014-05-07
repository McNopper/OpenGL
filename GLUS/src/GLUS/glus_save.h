/*
 * glus_save.h
 *
 *  Created on: May 7, 2014
 *      Author: B48616
 */

#ifndef GLUS_SAVE_H_
#define GLUS_SAVE_H_

/**
 * Saves a TGA file.
 *
 * @param filename The name of the file to save.
 * @param tgaimage The structure with the TGA data.
 *
 * @return GLUS_TRUE, if saving succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusSaveTgaImage(const GLUSchar* filename, const GLUStgaimage* tgaimage);

/**
 * Saves a HDR file.
 *
 * @param filename The name of the file to save.
 * @param hdrimage The structure with the HDR data.
 *
 * @return GLUS_TRUE, if saving succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusSaveHdrImage(const GLUSchar* filename, const GLUShdrimage* hdrimage);

/**
 * Saves a binary file.
 *
 * @param filename		The name of the file to save.
 * @param binaryfile 	The structure with the binary data.
 *
 * @return GLUS_TRUE, if saving succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusSaveBinaryFile(const GLUSchar* filename, const GLUSbinaryfile* binaryfile);

/**
 * Saves a text file.
 *
 * @param filename	The name of the file to save.
 * @param textfile 	The structure with the text data.
 *
 * @return GLUS_TRUE, if saving succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusSaveTextFile(const GLUSchar* filename, const GLUStextfile* textfile);

#endif /* GLUS_SAVE_H_ */
