/*
 * GLUS - OpenGL ES 2.0 and 3.0 Utilities. Copyright (C) 2010 - 2013 Norbert Nopper
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

extern GLUSvoid _glusPollEvents(GLUSvoid);

extern EGLNativeDisplayType _glusGetNativeDisplayType(GLUSvoid);

extern EGLNativeWindowType _glusCreateNativeWindowType(const char* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, const GLUSint nativeVisualID);

extern GLUSvoid _glusDestroyNativeWindowDisplay(GLUSvoid);

extern double _glusGetRawTime(GLUSvoid);

extern GLUSvoid _glusGetWindowSize(GLUSint* width, GLUSint* height);

static EGLDisplay g_eglDisplay = EGL_NO_DISPLAY;
static EGLDisplay g_eglSurface = EGL_NO_SURFACE;
static EGLDisplay g_eglContext = EGL_NO_CONTEXT;

static GLUSboolean g_windowCreated = GLUS_FALSE;
static GLUSboolean g_initdone = GLUS_FALSE;
static GLUSboolean g_done = GLUS_FALSE;
static GLUSint g_buttons = 0;
static GLUSint g_mouseX = 0;
static GLUSint g_mouseY = 0;

static GLUSint g_width = 640;
static GLUSint g_height = 480;

static GLUSboolean (*glusInit)(GLUSvoid) = 0;
static GLUSvoid (*glusReshape)(GLUSint width, GLUSint height) = 0;
static GLUSboolean (*glusUpdate)(GLUSfloat time) = 0;
static GLUSvoid (*glusTerminate)(GLUSvoid) = 0;

static GLUSvoid (*glusKey)(GLUSboolean pressed, GLUSint key) = 0;
static GLUSvoid (*glusMouse)(GLUSboolean pressed, GLUSint button, GLUSint xPos, GLUSint yPos) = 0;
static GLUSvoid (*glusMouseWheel)(GLUSint buttons, GLUSint ticks, GLUSint xPos, GLUSint yPos) = 0;
static GLUSvoid (*glusMouseMove)(GLUSint buttons, GLUSint xPos, GLUSint yPos) = 0;

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

	measuredTime = (GLUSfloat)_glusGetRawTime();

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

GLUSvoid glusInternalReshape(GLUSint width, GLUSint height)
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

GLUSint glusInternalClose(void)
{
	g_done = GLUS_TRUE;

	return 0;
}

GLUSvoid glusInternalKey(GLUSint key, GLUSint state)
{
	if (state == GLFW_RELEASE)
	{
		if (key == GLFW_KEY_ESCAPE)
		{
			g_done = GLUS_TRUE;

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

GLUSvoid glusInternalMouse(GLUSint button, GLUSint action)
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

GLUSvoid glusInternalMouseWheel(GLUSint pos)
{
	if (glusMouseWheel)
	{
		glusMouseWheel(g_buttons, pos, g_mouseX, g_mouseY);
	}
}

GLUSvoid glusInternalMouseMove(GLUSint x, GLUSint y)
{
	g_mouseX = x;
	g_mouseY = y;

	if (glusMouseMove)
	{
		glusMouseMove(g_buttons, g_mouseX, g_mouseY);
	}
}

GLUSvoid GLUSAPIENTRY glusDestroyWindow(GLUSvoid)
{
	glusEGLTerminate(&g_eglDisplay, &g_eglContext, &g_eglSurface);

	_glusDestroyNativeWindowDisplay();

	g_windowCreated = GLUS_FALSE;

	g_initdone = GLUS_FALSE;
}

GLUSboolean GLUSAPIENTRY glusCreateWindow(const GLUSchar* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, const EGLint* configAttribList, const EGLint* contextAttribList)
{
	EGLConfig eglConfig;

	EGLNativeWindowType eglNativeWindowType;

	EGLint nativeVisualID = 0;

	if (g_windowCreated)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Window already exists");

		return GLUS_FALSE;
	}

	if (!glusEGLCreateContext(_glusGetNativeDisplayType(), &g_eglDisplay, &eglConfig, &g_eglContext, configAttribList, contextAttribList))
	{
		glusDestroyWindow();

		return GLUS_FALSE;
	}

	eglGetConfigAttrib(g_eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, &nativeVisualID);

	eglNativeWindowType = _glusCreateNativeWindowType(title, width, height, fullscreen, noResize, nativeVisualID);

	if (!eglNativeWindowType)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Could not create native window");

		glusDestroyWindow();

		return GLUS_FALSE;
	}

	if (!glusEGLCreateWindowSurfaceMakeCurrent(eglNativeWindowType, &g_eglDisplay, &eglConfig, &g_eglContext, &g_eglSurface))
	{
		glusDestroyWindow();

		return GLUS_FALSE;
	}

	_glusGetWindowSize(&g_width, &g_height);

	g_windowCreated = GLUS_TRUE;

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
	if (!g_done) // Loop That Runs While done=FALSE
	{
		if (glusUpdate)
		{
			g_done = !glusUpdate(glusGetElapsedTime());
		}

		eglSwapBuffers(g_eglDisplay, g_eglSurface); // Swap Buffers (Double Buffering)

		_glusPollEvents();
	}

	return !g_done;
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
	return eglGetProcAddress(procname);
}

GLUSvoid GLUSAPIENTRY glusSwapInterval(GLUSint interval)
{
	eglSwapInterval(g_eglDisplay, interval);
}

//

EGLDisplay GLUSAPIENTRY glusEGLGetDefaultDisplay(GLUSvoid)
{
	return g_eglDisplay;
}

EGLSurface GLUSAPIENTRY glusEGLGetDefaultSurface(GLUSvoid)
{
	return g_eglSurface;
}

EGLContext GLUSAPIENTRY glusEGLGetDefaultContext(GLUSvoid)
{
	return g_eglContext;
}
