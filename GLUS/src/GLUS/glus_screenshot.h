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

#ifndef GLUS_SCREENSHOT_H_
#define GLUS_SCREENSHOT_H_

/**
 * Creates a screenshot of the current back buffer and uses the given image and its dimension.
 *
 * @param x The lower left x coordinate from the screenshot rectangle.
 * @param y The lower left y coordinate from the screenshot rectangle.
 * @param width The width of the screenshot rectangle.
 * @param height The height of the screenshot rectangle.
 * @param screenshot The image structure to store the pixel data.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusScreenshotUseTga(GLUSint x, GLUSint y, const GLUStgaimage* screenshot);

/**
 * Creates a screenshot of the current back buffer and creates the image automatically.
 *
 * @param x The lower left x coordinate from the screenshot rectangle.
 * @param y The lower left y coordinate from the screenshot rectangle.
 * @param width The width of the screenshot rectangle.
 * @param height The height of the screenshot rectangle.
 * @param screenshot The image structure to store the pixel data.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusScreenshotCreateTga(GLUSint x, GLUSint y, GLUSsizei width, GLUSsizei height, GLUStgaimage* screenshot);

#endif /* GLUS_SCREENSHOT_H_ */
