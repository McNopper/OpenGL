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

GLUSvoid GLUSAPIENTRY glusKeyFunc(GLUSvoid (*glusNewKey)(GLUSboolean pressed, GLUSint key))
{
	glusKey = glusNewKey;
}

GLUSvoid GLUSAPIENTRY glusMouseFunc(GLUSvoid (*glusNewMouse)(GLUSboolean pressed, GLUSint button, GLUSint xPos, GLUSint yPos))
{
	glusMouse = glusNewMouse;
}

GLUSvoid GLUSAPIENTRY glusMouseWheelFunc(GLUSvoid (*glusNewMouseWheel)(GLUSint buttons, GLUSint ticks, GLUSint xPos, GLUSint yPos))
{
	glusMouseWheel = glusNewMouseWheel;
}

GLUSvoid GLUSAPIENTRY glusMouseMoveFunc(GLUSvoid (*glusNewMouseMove)(GLUSint buttons, GLUSint xPos, GLUSint yPos))
{
	glusMouseMove = glusNewMouseMove;
}

GLUSvoid GLUSAPIENTRY glusInitFunc(GLUSboolean (*glusNewInit)(GLUSvoid))
{
	glusInit = glusNewInit;
}

GLUSvoid GLUSAPIENTRY glusReshapeFunc(GLUSvoid (*glusNewReshape)(GLUSint width, GLUSint height))
{
	glusReshape = glusNewReshape;
}

GLUSvoid GLUSAPIENTRY glusUpdateFunc(GLUSboolean (*glusNewUpdate)(GLUSfloat time))
{
	glusUpdate = glusNewUpdate;
}

GLUSvoid GLUSAPIENTRY glusTerminateFunc(GLUSvoid (*glusNewTerminate)(GLUSvoid))
{
	glusTerminate = glusNewTerminate;
}

static GLUSfloat glusGetElapsedTime(GLUSvoid)
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

static GLUSvoid glusInternalReshape(GLFWwindow* window, GLUSint width, GLUSint height)
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

static GLUSvoid glusInternalClose(GLFWwindow* window)
{
	glfwSetWindowShouldClose(window, GLUS_TRUE);
}

static GLUSvoid glusInternalKey(GLFWwindow* window, GLUSint key, GLUSint scancode, GLUSint action, GLUSint mods)
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

static GLUSvoid glusInternalMouse(GLFWwindow* window, GLUSint button, GLUSint action, GLUSint mods)
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

static GLUSvoid glusInternalMouseWheel(GLFWwindow* window, double xpos, double ypos)
{
	static GLUSint wheelPos = 0;

	if (glusMouseWheel)
	{
		wheelPos += (GLUSint)ypos;

		glusMouseWheel(g_buttons, wheelPos, g_mouseX, g_mouseY);
	}
}

static GLUSvoid glusInternalMouseMove(GLFWwindow* window, double x, double y)
{
	g_mouseX = (GLUSint)x;
	g_mouseY = (GLUSint)y;

	if (glusMouseMove)
	{
		glusMouseMove(g_buttons, g_mouseX, g_mouseY);
	}
}

static GLUSvoid GLUSAPIENTRY glusInternalDebugMessage(GLUSenum source, GLUSenum type, GLUSuint id, GLUSenum severity, GLUSsizei length, const GLUSchar* message, const GLUSvoid* userParam)
{
	glusLogPrint(GLUS_LOG_DEBUG, "source: 0x%04X type: 0x%04X id: %u severity: 0x%04X '%s'", source, type, id, severity, message);
}

GLUSvoid GLUSAPIENTRY glusDestroyWindow(GLUSvoid)
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

GLUSboolean GLUSAPIENTRY glusCreateWindow(const GLUSchar* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, const EGLint* configAttribList, const EGLint* contextAttribList)
{
	int major = 3;
	int minor = 2;
	int forward = 1;
	int profile = GLFW_OPENGL_CORE_PROFILE;
	int samples = 0;
	int redBits = 8;
	int greenBits = 8;
	int blueBits = 8;
	int depthBits = 0;
	int stencilBits = 0;
	int alphaBits = 0;
	int debug = 0;

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

	walker = configAttribList;
	while(walker && *walker != EGL_NONE)
	{
		switch(*walker)
		{
			case EGL_RENDERABLE_TYPE:
				if (*(walker + 1) != EGL_OPENGL_BIT)
				{
					glusLogPrint(GLUS_LOG_ERROR, "EGL_RENDERABLE_TYPE has to be EGL_OPENGL_BIT");

					return GLUS_FALSE;
				}
				break;
			case EGL_RED_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE)
				{
					redBits = *(walker + 1);
				}
				break;
			case EGL_GREEN_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE)
				{
					greenBits = *(walker + 1);
				}
				break;
			case EGL_BLUE_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE)
				{
					blueBits = *(walker + 1);
				}
				break;
			case EGL_DEPTH_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE)
				{
					depthBits = *(walker + 1);
				}
				break;
			case EGL_STENCIL_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE)
				{
					stencilBits = *(walker + 1);
				}
				break;
			case EGL_ALPHA_SIZE:
				if (*(walker + 1) != EGL_DONT_CARE)
				{
					alphaBits = *(walker + 1);
				}
				break;
			case EGL_SAMPLES:
				samples = *(walker + 1);
				break;
		}

		walker += 2;
	}

	walker = contextAttribList;
	while(walker && *walker != EGL_NONE)
	{
		switch(*walker)
		{
			case EGL_CONTEXT_MAJOR_VERSION:
				major = *(walker + 1);
				break;
			case EGL_CONTEXT_MINOR_VERSION:
				minor = *(walker + 1);
				break;
			case EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE:
				if (*(walker + 1) == EGL_TRUE)
				{
					forward = 1;
				}
				else if (*(walker + 1) == EGL_FALSE)
				{
					forward = 0;
				}
				break;
			case EGL_CONTEXT_OPENGL_PROFILE_MASK:
				if (*(walker + 1) & EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT)
				{
					profile = GLFW_OPENGL_CORE_PROFILE;
				}
				else if (*(walker + 1) & EGL_CONTEXT_OPENGL_COMPATIBILITY_PROFILE_BIT)
				{
					profile = GLFW_OPENGL_COMPAT_PROFILE;
				}
				break;
			case EGL_CONTEXT_OPENGL_DEBUG:
				if (*(walker + 1) == EGL_TRUE)
				{
					debug = 1;
				}
				else if (*(walker + 1) == EGL_FALSE)
				{
					debug = 0;
				}
				break;
		}

		walker += 2;
	}

	glfwWindowHint(GLFW_RED_BITS, redBits);
	glfwWindowHint(GLFW_GREEN_BITS, greenBits);
	glfwWindowHint(GLFW_BLUE_BITS, blueBits);
	glfwWindowHint(GLFW_DEPTH_BITS, depthBits);
	glfwWindowHint(GLFW_STENCIL_BITS, stencilBits);
	glfwWindowHint(GLFW_ALPHA_BITS, alphaBits);

	glfwWindowHint(GLFW_SAMPLES, samples);

	//

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, forward);
	glfwWindowHint(GLFW_OPENGL_PROFILE, profile);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, debug);

	//

	glfwWindowHint(GLFW_RESIZABLE, !noResize);

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
		glusDestroyWindow();

		glusLogPrint(GLUS_LOG_ERROR, "GLEW could not be initialized: %x", err);

		return GLUS_FALSE;
	}

	// Catch all OpenGL errors so far.
	glGetError();

	if (!glusIsSupported(major, minor))
	{
		glusDestroyWindow();

		glusLogPrint(GLUS_LOG_ERROR, "OpenGL %u.%u not supported", major, minor);

		return GLUS_FALSE;
	}

	glfwSetWindowSizeCallback(g_window, glusInternalReshape);
	glfwSetWindowCloseCallback(g_window, glusInternalClose);
	glfwSetKeyCallback(g_window, glusInternalKey);
	glfwSetMouseButtonCallback(g_window, glusInternalMouse);
	glfwSetScrollCallback(g_window, glusInternalMouseWheel);
	glfwSetCursorPosCallback(g_window, glusInternalMouseMove);

	glfwGetWindowSize(g_window, &g_width, &g_height);

	if (debug && glusIsSupported(4, 3))
	{
		glusLogSetLevel(GLUS_LOG_DEBUG);

		glDebugMessageCallback(&glusInternalDebugMessage, 0);
	}

	return GLUS_TRUE; // Success
}

GLUSboolean GLUSAPIENTRY glusRun(GLUSvoid)
{
	if (!glusStartup())
	{
		return GLUS_FALSE;
	}

	while (glusLoop())
	{
		// Do nothing here
	}

	glusShutdown();

	return GLUS_TRUE; // Exit The Program
}

GLUSboolean GLUSAPIENTRY glusStartup(GLUSvoid)
{
	// Init Engine
	if (glusInit)
	{
		if (!glusInit())
		{
			glusShutdown();

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

GLUSboolean GLUSAPIENTRY glusLoop(GLUSvoid)
{
	if (!glfwWindowShouldClose(g_window))
	{
		if (glusUpdate)
		{
			if (!glusUpdate(glusGetElapsedTime()))
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

GLUSvoid GLUSAPIENTRY glusShutdown(GLUSvoid)
{
	// Terminate Game
	if (glusTerminate)
	{
		glusTerminate();
	}

	// Shutdown
	glusDestroyWindow(); // Destroy The Window
}

void* GLUSAPIENTRY glusGetProcAddress(const GLUSchar* procname)
{
	return glfwGetProcAddress(procname);
}

GLUSvoid GLUSAPIENTRY glusSwapInterval(GLUSint interval)
{
	glfwSwapInterval(interval);
}
