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

extern GLUSint _glusWindowGetCurrentRecordingFrame(GLUSvoid);

extern GLUSint _glusWindowGetCurrentAndIncreaseRecordingFrame(GLUSvoid);

extern GLUSint _glusWindowGetRecordingFrames(GLUSvoid);

extern GLUSfloat _glusWindowGetRecordingTime(GLUSvoid);

extern const GLUStgaimage* _glusWindowGetRecordingImageTga(GLUSvoid);

static GLFWwindow* g_window = 0;
static GLUSboolean g_initdone = GLUS_FALSE;
static GLUSint g_buttons = 0;
static GLUSint g_mouseX = 0;
static GLUSint g_mouseY = 0;

static GLUSint g_width = 640;
static GLUSint g_height = 480;

static GLUSboolean (*glusInit)(GLUSvoid) = NULL;
static GLUSvoid (*glusReshape)(GLUSint width, GLUSint height) = NULL;
static GLUSboolean (*glusUpdate)(GLUSfloat time) = NULL;
static GLUSvoid (*glusTerminate)(GLUSvoid) = NULL;

static GLUSvoid (*glusKey)(GLUSboolean pressed, GLUSint key) = NULL;
static GLUSvoid (*glusMouse)(GLUSboolean pressed, GLUSint button, GLUSint xPos, GLUSint yPos) = NULL;
static GLUSvoid (*glusMouseWheel)(GLUSint buttons, GLUSint ticks, GLUSint xPos, GLUSint yPos) = NULL;
static GLUSvoid (*glusMouseMove)(GLUSint buttons, GLUSint xPos, GLUSint yPos) = NULL;

GLUSvoid GLUSAPIENTRY glusWindowSetKeyFunc(GLUSvoid (*glusNewKey)(GLUSboolean pressed, GLUSint key))
{
	glusKey = glusNewKey;
}

GLUSvoid GLUSAPIENTRY glusWindowSetMouseFunc(GLUSvoid (*glusNewMouse)(GLUSboolean pressed, GLUSint button, GLUSint xPos, GLUSint yPos))
{
	glusMouse = glusNewMouse;
}

GLUSvoid GLUSAPIENTRY glusWindowSetMouseWheelFunc(GLUSvoid (*glusNewMouseWheel)(GLUSint buttons, GLUSint ticks, GLUSint xPos, GLUSint yPos))
{
	glusMouseWheel = glusNewMouseWheel;
}

GLUSvoid GLUSAPIENTRY glusWindowSetMouseMoveFunc(GLUSvoid (*glusNewMouseMove)(GLUSint buttons, GLUSint xPos, GLUSint yPos))
{
	glusMouseMove = glusNewMouseMove;
}

GLUSvoid GLUSAPIENTRY glusWindowSetInitFunc(GLUSboolean (*glusNewInit)(GLUSvoid))
{
	glusInit = glusNewInit;
}

GLUSvoid GLUSAPIENTRY glusWindowSetReshapeFunc(GLUSvoid (*glusNewReshape)(GLUSint width, GLUSint height))
{
	glusReshape = glusNewReshape;
}

GLUSvoid GLUSAPIENTRY glusWindowSetUpdateFunc(GLUSboolean (*glusNewUpdate)(GLUSfloat time))
{
	glusUpdate = glusNewUpdate;
}

GLUSvoid GLUSAPIENTRY glusWindowSetTerminateFunc(GLUSvoid (*glusNewTerminate)(GLUSvoid))
{
	glusTerminate = glusNewTerminate;
}

static GLUSfloat glusWindowGetElapsedTime(GLUSvoid)
{
	static GLUSboolean init = GLUS_FALSE;
	static GLUSfloat lastTime;
	static GLUSfloat currentTime;

	GLUSfloat measuredTime;

	measuredTime = (GLUSfloat)glfwGetTime();

	if (!init)
	{
		lastTime = measuredTime;

		currentTime = measuredTime;

		init = GLUS_TRUE;
	}
	else
	{
		lastTime = currentTime;

		currentTime = measuredTime;
	}

	return currentTime - lastTime;
}

GLUSvoid _glusWindowInternalReshape(GLFWwindow* window, GLUSint width, GLUSint height)
{
	if (width < 1)
	{
		width = 1;
	}
	if (height < 1)
	{
		height = 1;
	}

	if (glusReshape && g_initdone)
	{
		glusReshape(width, height);
	}
}

GLUSvoid _glusWindowInternalClose(GLFWwindow* window)
{
	glfwSetWindowShouldClose(window, GLUS_TRUE);
}

GLUSvoid _glusWindowInternalKey(GLFWwindow* window, GLUSint key, GLUSint scancode, GLUSint action, GLUSint mods)
{
	if (action == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_ESCAPE)
		{
			glfwSetWindowShouldClose(window, GLUS_TRUE);

			return;
		}

		if (glusKey)
		{
			glusKey(GLUS_FALSE, tolower(key));
		}
	}
	else
	{
		if (glusKey)
		{
			glusKey(GLUS_TRUE, tolower(key));
		}
	}
}

GLUSvoid _glusWindowInternalMouse(GLFWwindow* window, GLUSint button, GLUSint action, GLUSint mods)
{
	GLUSint usedButton = 0;

	if (button == GLFW_MOUSE_BUTTON_LEFT)
	{
		usedButton = 1;
	}
	else if (button == GLFW_MOUSE_BUTTON_MIDDLE)
	{
		usedButton = 2;
	}
	else if (button == GLFW_MOUSE_BUTTON_RIGHT)
	{
		usedButton = 4;
	}

	if (action == GLFW_PRESS)
	{
		g_buttons |= usedButton;
	}
	else
	{
		g_buttons ^= usedButton;
	}

	if (glusMouse)
	{
		glusMouse(action == GLFW_PRESS, usedButton, g_mouseX, g_mouseY);
	}
}

GLUSvoid _glusWindowInternalMouseWheel(GLFWwindow* window, double xpos, double ypos)
{
	static GLUSint wheelPos = 0;

	if (glusMouseWheel)
	{
		wheelPos += (GLUSint)ypos;

		glusMouseWheel(g_buttons, wheelPos, g_mouseX, g_mouseY);
	}
}

GLUSvoid _glusWindowInternalMouseMove(GLFWwindow* window, double x, double y)
{
	g_mouseX = (GLUSint)x;
	g_mouseY = (GLUSint)y;

	if (glusMouseMove)
	{
		glusMouseMove(g_buttons, g_mouseX, g_mouseY);
	}
}

static GLUSvoid GLUSAPIENTRY glusWindowDebugMessage(GLUSenum source, GLUSenum type, GLUSuint id, GLUSenum severity, GLUSsizei length, const GLUSchar* message, const GLUSvoid* userParam)
{
	glusLogPrint(GLUS_LOG_DEBUG, "source: 0x%04X type: 0x%04X id: %u severity: 0x%04X '%s'", source, type, id, severity, message);
}

GLUSvoid GLUSAPIENTRY glusWindowDestroy(GLUSvoid)
{
	if (g_window)
	{
		glfwMakeContextCurrent(0);

		glfwDestroyWindow(g_window);

		g_window = 0;
	}

	glfwTerminate();

	g_initdone = GLUS_FALSE;
}

GLUSboolean GLUSAPIENTRY glusWindowCreate(const GLUSchar* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, const EGLint* configAttribList, const EGLint* contextAttribList)
{
	int samples = 0;

	GLUSboolean eglRenderableTypeProcessed = GLUS_FALSE;
	GLUSboolean eglSampleBuffersProcessed = GLUS_FALSE;
	GLUSboolean eglSamplesProcessed = GLUS_FALSE;

	int major = 3;
	int minor = 2;
	int profile = GLFW_OPENGL_CORE_PROFILE;
	int debug = 0;

	GLUSboolean eglContextMajorVersionProcessed = GLUS_FALSE;
	GLUSboolean eglContextMinorVersionProcessed = GLUS_FALSE;

	GLUSenum err;

	const EGLint* walker;

	if (g_window)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Window already exists");

		return GLUS_FALSE;
	}

	if (!glfwInit())
	{
		glusLogPrint(GLUS_LOG_ERROR, "GLFW could not be initialized");

		return GLUS_FALSE;
	}

	//

	glfwWindowHint(GLFW_RESIZABLE, !noResize);

	//

	walker = configAttribList;
	while(walker && *walker != EGL_NONE)
	{
		switch(*walker)
		{
			case EGL_RENDERABLE_TYPE:
				if (*(walker + 1) == EGL_OPENGL_BIT)
				{
					eglRenderableTypeProcessed = GLUS_TRUE;
				}
				else
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_RENDERABLE_TYPE has to be EGL_OPENGL_BIT");

					return GLUS_FALSE;
				}
				break;
			case EGL_RED_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE && *(walker + 1) >= 0)
				{
					glfwWindowHint(GLFW_RED_BITS, *(walker + 1));
				}
				else if (*(walker + 1) != EGL_DONT_CARE)
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_RED_SIZE has to be >= 0");

					return GLUS_FALSE;
				}
				break;
			case EGL_GREEN_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE && *(walker + 1) >= 0)
				{
					glfwWindowHint(GLFW_GREEN_BITS, *(walker + 1));
				}
				else if (*(walker + 1) != EGL_DONT_CARE)
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_GREEN_SIZE has to be >= 0");

					return GLUS_FALSE;
				}
				break;
			case EGL_BLUE_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE && *(walker + 1) >= 0)
				{
					glfwWindowHint(GLFW_BLUE_BITS, *(walker + 1));
				}
				else if (*(walker + 1) != EGL_DONT_CARE)
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_BLUE_SIZE has to be >= 0");

					return GLUS_FALSE;
				}
				break;
			case EGL_DEPTH_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE && *(walker + 1) >= 0)
				{
					glfwWindowHint(GLFW_DEPTH_BITS, *(walker + 1));
				}
				else if (*(walker + 1) != EGL_DONT_CARE)
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_DEPTH_SIZE has to be >= 0");

					return GLUS_FALSE;
				}
				break;
			case EGL_STENCIL_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE && *(walker + 1) >= 0)
				{
					glfwWindowHint(GLFW_STENCIL_BITS, *(walker + 1));
				}
				else if (*(walker + 1) != EGL_DONT_CARE)
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_STENCIL_SIZE has to be >= 0");

					return GLUS_FALSE;
				}
				break;
			case EGL_ALPHA_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE && *(walker + 1) >= 0)
				{
					glfwWindowHint(GLFW_ALPHA_BITS, *(walker + 1));
				}
				else if (*(walker + 1) != EGL_DONT_CARE)
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_ALPHA_SIZE has to be >= 0");

					return GLUS_FALSE;
				}
				break;
			case EGL_SAMPLE_BUFFERS:
				if (*(walker + 1) >= 0 && *(walker + 1) <= 1)
				{
					if (eglSamplesProcessed && *(walker + 1) == 0)
					{
						samples = 0;
					}
					else if (!eglSamplesProcessed)
					{
						samples = *(walker + 1);
					}

					eglSampleBuffersProcessed = GLUS_TRUE;
				}
				else
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_SAMPLE_BUFFERS has to be >= 0 and <= 1");

					return GLUS_FALSE;
				}
				break;
			case EGL_SAMPLES:
				if (*(walker + 1) >= 0)
				{
					if ((eglSampleBuffersProcessed && samples == 1) || !eglSampleBuffersProcessed)
					{
						samples = *(walker + 1);
					}

					eglSamplesProcessed = GLUS_TRUE;
				}
				else
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_SAMPLES has to be >= 0");

					return GLUS_FALSE;
				}
				break;
		}

		walker += 2;
	}

	if (!eglRenderableTypeProcessed)
	{
		glusLogPrint(GLUS_LOG_ERROR, "EGL_RENDERABLE_TYPE not specified");

		return GLUS_FALSE;
	}

	if ((eglSampleBuffersProcessed && !eglSamplesProcessed) || (!eglSampleBuffersProcessed && eglSamplesProcessed))
	{
		glusLogPrint(GLUS_LOG_ERROR, "EGL_SAMPLE_BUFFERS and EGL_SAMPLES has to be specified");

		return GLUS_FALSE;
	}

	glfwWindowHint(GLFW_SAMPLES, samples);
	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);

	//

	walker = contextAttribList;
	while(walker && *walker != EGL_NONE)
	{
		switch(*walker)
		{
			case EGL_CONTEXT_MAJOR_VERSION:
				if (*(walker + 1) >= 1)
				{
					major = *(walker + 1);

					glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);

					eglContextMajorVersionProcessed = GLUS_TRUE;
				}
				else
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_CONTEXT_MAJOR_VERSION has to be >= 1");

					return GLUS_FALSE;
				}
				break;
			case EGL_CONTEXT_MINOR_VERSION:
				if (*(walker + 1) >= 0)
				{
					minor = *(walker + 1);

					glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);

					eglContextMinorVersionProcessed = GLUS_TRUE;
				}
				else
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_CONTEXT_MINOR_VERSION has to be >= 0");

					return GLUS_FALSE;
				}
				break;
			case EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE:
				if (*(walker + 1) == EGL_TRUE || *(walker + 1) == EGL_FALSE)
				{
					glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, *(walker + 1));
				}
				else
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE is invalid");

					return GLUS_FALSE;
				}
				break;
			case EGL_CONTEXT_OPENGL_PROFILE_MASK:
				if (*(walker + 1) == EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT)
				{
					profile = GLFW_OPENGL_CORE_PROFILE;
				}
				else if (*(walker + 1) == EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT)
				{
					profile = GLFW_OPENGL_COMPAT_PROFILE;
				}
				else
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_CONTEXT_OPENGL_PROFILE_MASK is invalid");

					return GLUS_FALSE;
				}
				break;
			case EGL_CONTEXT_OPENGL_DEBUG:
				if (*(walker + 1) == EGL_TRUE || *(walker + 1) == EGL_FALSE)
				{
					debug = *(walker + 1);

					glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, debug);
				}
				else
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_CONTEXT_OPENGL_DEBUG is invalid");

					return GLUS_FALSE;
				}
				break;
		}

		walker += 2;
	}

	if (!eglContextMajorVersionProcessed || !eglContextMinorVersionProcessed)
	{
		glusLogPrint(GLUS_LOG_ERROR, "EGL_CONTEXT_CLIENT_VERSION, EGL_CONTEXT_MAJOR_VERSION or EGL_CONTEXT_MINOR_VERSION not specified");

		return GLUS_FALSE;
	}

	if (major <= 3 && minor <= 1)
	{
		profile = GLFW_OPENGL_ANY_PROFILE;
	}

	glfwWindowHint(GLFW_OPENGL_PROFILE, profile);

	//

	g_window = glfwCreateWindow(width, height, title, fullscreen ? glfwGetPrimaryMonitor() : 0, 0);
	if (!g_window)
	{
		glfwTerminate();

		glusLogPrint(GLUS_LOG_ERROR, "GLFW window could not be opened");

		return GLUS_FALSE;
	}

	glfwMakeContextCurrent(g_window);

	glewExperimental = GLUS_TRUE;

	err = glewInit();

	if (GLUS_OK != err)
	{
		glusWindowDestroy();

		glusLogPrint(GLUS_LOG_ERROR, "GLEW could not be initialized: %x", err);

		return GLUS_FALSE;
	}

	// Catch all OpenGL errors so far.
	glGetError();

	if (!glusVersionIsSupported(major, minor))
	{
		glusWindowDestroy();

		glusLogPrint(GLUS_LOG_ERROR, "OpenGL %u.%u not supported", major, minor);

		return GLUS_FALSE;
	}

	glfwSetWindowSizeCallback(g_window, _glusWindowInternalReshape);
	glfwSetWindowCloseCallback(g_window, _glusWindowInternalClose);
	glfwSetKeyCallback(g_window, _glusWindowInternalKey);
	glfwSetMouseButtonCallback(g_window, _glusWindowInternalMouse);
	glfwSetScrollCallback(g_window, _glusWindowInternalMouseWheel);
	glfwSetCursorPosCallback(g_window, _glusWindowInternalMouseMove);

	glfwGetWindowSize(g_window, &g_width, &g_height);

	if (debug && glusVersionIsSupported(4, 3))
	{
		glusLogSetLevel(GLUS_LOG_DEBUG);

		glDebugMessageCallback(&glusWindowDebugMessage, 0);
	}

	return GLUS_TRUE; // Success
}

GLUSboolean GLUSAPIENTRY glusWindowRun(GLUSvoid)
{
	GLUSboolean run = GLUS_TRUE;

	if (!glusWindowStartup())
	{
		return GLUS_FALSE;
	}

	while (run)
	{
		if (glusWindowIsRecording())
		{
			run = glusWindowLoopDoRecording();
		}
		else
		{
			run = glusWindowLoop();
		}
	}

	glusWindowShutdown();

	return GLUS_TRUE; // Exit The Program
}

GLUSboolean GLUSAPIENTRY glusWindowStartup(GLUSvoid)
{
	// Init Engine
	if (glusInit)
	{
		if (!glusInit())
		{
			glusWindowShutdown();

			return GLUS_FALSE; // Exit The Program
		}
	}

	g_initdone = GLUS_TRUE;

	// Do the first reshape
	if (glusReshape)
	{
		glusReshape(g_width, g_height);
	}

	return GLUS_TRUE;
}

GLUSboolean GLUSAPIENTRY glusWindowLoop(GLUSvoid)
{
	if (!glfwWindowShouldClose(g_window))
	{
		if (glusUpdate)
		{
			if (!glusUpdate(glusWindowGetElapsedTime()))
			{
				glfwSetWindowShouldClose(g_window, GLUS_TRUE);
			}
		}

		glfwSwapBuffers(g_window); // Swap Buffers

		glfwPollEvents();

		return GLUS_TRUE;
	}

	return GLUS_FALSE;
}

GLUSboolean GLUSAPIENTRY glusWindowLoopDoRecording(GLUSvoid)
{
	if (!glfwWindowShouldClose(g_window))
	{
		if (glusUpdate)
		{
			// Still consume and update time, as a fixed recording time is used.
			glusWindowGetElapsedTime();

			if (!glusUpdate(_glusWindowGetRecordingTime()))
			{
				glfwSetWindowShouldClose(g_window, GLUS_TRUE);
			}
			else
			{
				if (_glusWindowGetCurrentRecordingFrame() < _glusWindowGetRecordingFrames())
				{
					if (glusScreenshotUseTga(0, 0, _glusWindowGetRecordingImageTga()))
					{
						static const GLUSchar* filenameTemplate = "screenshot-%04d.tga";
						static GLUSchar filename[20];

						sprintf(filename, filenameTemplate, _glusWindowGetCurrentAndIncreaseRecordingFrame());

						glusImageSaveTga(filename, _glusWindowGetRecordingImageTga());
					}
				}
				else
				{
					glfwSetWindowShouldClose(g_window, GLUS_TRUE);
				}
			}
		}

		glfwSwapBuffers(g_window); // Swap Buffers

		glfwPollEvents();

		return GLUS_TRUE;
	}

	return GLUS_FALSE;
}

GLUSvoid GLUSAPIENTRY glusWindowShutdown(GLUSvoid)
{
	// Terminate Game
	if (glusTerminate)
	{
		glusTerminate();
	}

	if (glusWindowIsRecording())
	{
		glusWindowStopRecording();
	}

	// Shutdown
	glusWindowDestroy(); // Destroy The Window
}

GLUSvoid GLUSAPIENTRY glusWindowSwapInterval(GLUSint interval)
{
	glfwSwapInterval(interval);
}

GLUSint GLUSAPIENTRY glusWindowGetWidth(GLUSvoid)
{
	return g_width;
}

GLUSint GLUSAPIENTRY glusWindowGetHeight(GLUSvoid)
{
	return g_height;
}

//

void* GLUSAPIENTRY glusExtensionGetFuncAddress(const GLUSchar* procname)
{
	return glfwGetProcAddress(procname);
}
