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

#define GLUS_MAX_CHARS_LOGGING 2047

static const char* GLUS_LOG_STRINGS[] = {
		"",
		"ERROR",
		"WARNING",
		"INFO",
		"DEBUG",
		"SEVERE"
};

static GLUSuint g_verbosity = GLUS_LOG_INFO;

GLUSvoid GLUSAPIENTRY glusLogSetLevel(const GLUSuint verbosity)
{
    g_verbosity = verbosity;
}

GLUSuint GLUSAPIENTRY glusLogGetLevel()
{
    return g_verbosity;
}

GLUSvoid GLUSAPIENTRY glusLogPrint(GLUSuint verbosity, const char* format, ...)
{
	if (g_verbosity == GLUS_LOG_NOTHING || verbosity == GLUS_LOG_NOTHING)
	{
		return;
	}

	if (g_verbosity >= verbosity)
	{
		const char* logString = "UNKNOWN";
		char buffer[GLUS_MAX_CHARS_LOGGING + 1];
		va_list argList;

		if (verbosity > GLUS_LOG_NOTHING && verbosity <= GLUS_LOG_SEVERE)
		{
			logString = GLUS_LOG_STRINGS[verbosity];
		}

		buffer[GLUS_MAX_CHARS_LOGGING] = '\0';

		va_start(argList, format);

		vsnprintf(buffer, GLUS_MAX_CHARS_LOGGING, format, argList);

		printf("LOG [%s]: %s\n", logString, buffer);

		va_end(argList);
	}
}

GLUSvoid GLUSAPIENTRY glusLogPrintError(GLUSuint verbosity, const char* format, ...)
{
	GLUSenum error;

	error = glGetError();

	if (g_verbosity == GLUS_LOG_NOTHING || verbosity == GLUS_LOG_NOTHING)
	{
		return;
	}

	if (verbosity < GLUS_LOG_DEBUG && error == GL_NO_ERROR)
	{
		return;
	}

	if (g_verbosity >= verbosity)
	{
		const char* logString = "UNKNOWN";
		char buffer[GLUS_MAX_CHARS_LOGGING + 1];
		va_list argList;

		if (verbosity > GLUS_LOG_NOTHING && verbosity <= GLUS_LOG_SEVERE)
		{
			logString = GLUS_LOG_STRINGS[verbosity];
		}

		buffer[GLUS_MAX_CHARS_LOGGING] = '\0';

		va_start(argList, format);

		vsnprintf(buffer, GLUS_MAX_CHARS_LOGGING, format, argList);

		printf("LOG [%s]: glGetError() = 0x%x %s\n", logString, error, buffer);

		va_end(argList);
	}
}
