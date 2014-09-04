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

#define GLUS_MAX_FRAMES 10000
#define GLUS_MAX_FRAMES_PER_SECOND 120

static GLUSint g_currentFrame = 0;
static GLUSint g_numberFrames = 0;
static GLUSfloat g_recordingTime = 0.0f;

static GLUStgaimage g_tgaimage = {0, 0, 0, 0, 0};

GLUSint _glusWindowGetCurrentRecordingFrame(GLUSvoid)
{
	return g_currentFrame;
}

GLUSint _glusWindowGetCurrentAndIncreaseRecordingFrame(GLUSvoid)
{
	return g_currentFrame++;
}

GLUSint _glusWindowGetRecordingFrames(GLUSvoid)
{
	return g_numberFrames;
}

GLUSfloat _glusWindowGetRecordingTime(GLUSvoid)
{
	return g_recordingTime;
}

const GLUStgaimage* _glusWindowGetRecordingImageTga(GLUSvoid)
{
	return &g_tgaimage;
}

GLUSboolean GLUSAPIENTRY glusWindowStartRecording(GLUSint numberFrames, GLUSint framesPerSecond)
{
	glusWindowStopRecording();

	if (numberFrames < 1 || numberFrames > GLUS_MAX_FRAMES || framesPerSecond < 1 || framesPerSecond > GLUS_MAX_FRAMES_PER_SECOND)
	{
		return GLUS_FALSE;
	}

	if (!glusImageCreateTga(&g_tgaimage, glusWindowGetWidth(), glusWindowGetHeight(), 1, GLUS_RGBA))
	{
		return GLUS_FALSE;
	}

	g_numberFrames = numberFrames;

	g_recordingTime = 1.0f / (GLUSfloat)framesPerSecond;

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusWindowIsRecording(GLUSvoid)
{
	return g_tgaimage.data != 0;
}

GLUSvoid GLUSAPIENTRY glusWindowStopRecording(GLUSvoid)
{
	g_currentFrame = 0;
	g_numberFrames = 0;
	g_recordingTime = 0.0f;

	glusImageDestroyTga(&g_tgaimage);
}
