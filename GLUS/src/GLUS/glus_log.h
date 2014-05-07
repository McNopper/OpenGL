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

#ifndef GLUS_LOG_H_
#define GLUS_LOG_H_

/**
 * Sets the current log level.
 *
 * @param verbosity The new log level.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusLogSetLevel(const GLUSuint verbosity);

/**
 * Gets the current log level.
 *
 * @return The current log level.
 */
GLUSAPI GLUSuint GLUSAPIENTRY glusLogGetLevel();

/**
 * Prints out the given information to the log console.
 *
 * @param verbosity The used log level.
 * @param format String to log plus format information.
 * @param ... Parameter list for the format string.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusLogPrint(GLUSuint verbosity, const char* format, ...);

/**
 * Executes glGetError() and prints out the error code plus the given information to the log console.
 *
 * @param verbosity The used log level.
 * @param format String to log plus format information.
 * @param ... Parameter list for the format string.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusLogPrintError(GLUSuint verbosity, const char* format, ...);

#endif /* GLUS_LOG_H_ */
