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

static GLUSfloat passedTime = 0.0f;
static GLUSfloat passedFrames = 0.0f;

GLUSvoid GLUSAPIENTRY glusProfileResetFPSf()
{
	passedTime = 0.0f;
	passedFrames = 0.0f;
}

GLUSvoid GLUSAPIENTRY glusProfileUpdateFPSf(GLUSfloat time)
{
	passedTime += time;

	if (passedTime >= 1.0f)
	{
		glusLogPrint(GLUS_LOG_INFO, "FPS: %d", (GLUSint)(passedFrames / passedTime));

		glusProfileResetFPSf();
	}

	passedFrames += 1.0f;
}

