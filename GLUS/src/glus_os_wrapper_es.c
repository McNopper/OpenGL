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

#ifdef GLUS_ES2
#include "GLES2/glus.h"
#else
#include "GLES3/glus.h"
#endif

extern GLUSvoid _glusPollEvents(GLUSvoid);

extern EGLNativeDisplayType _glusGetNativeDisplayType(GLUSvoid);

extern EGLNativeWindowType _glusCreateNativeWindowType(const char* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, EGLint eglNativeVisualID);

extern GLUSvoid _glusDestroyNativeWindow(GLUSvoid);

extern double _glusGetRawTime(GLUSvoid);

extern GLUSvoid _glusGetWindowSize(GLUSint* width, GLUSint* height);

static EGLint g_eglContextClientVersion = GLUS_DEFAULT_CLIENT_VERSION;

static EGLDisplay g_eglDisplay = EGL_NO_DISPLAY;
static EGLDisplay g_eglSurface = EGL_NO_SURFACE;
static EGLDisplay g_eglContext = EGL_NO_CONTEXT;

static GLUSboolean g_noResize = GLUS_FALSE;

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

GLUSvoid GLUSAPIENTRY glusKeyFunc(GLUSvoid(*glusNewKey)(GLUSboolean pressed, GLUSint key))
{
    glusKey = glusNewKey;
}

GLUSvoid GLUSAPIENTRY glusMouseFunc(GLUSvoid(*glusNewMouse)(GLUSboolean pressed, GLUSint button, GLUSint xPos, GLUSint yPos))
{
    glusMouse = glusNewMouse;
}

GLUSvoid GLUSAPIENTRY glusMouseWheelFunc(GLUSvoid(*glusNewMouseWheel)(GLUSint buttons, GLUSint ticks, GLUSint xPos, GLUSint yPos))
{
    glusMouseWheel = glusNewMouseWheel;
}

GLUSvoid GLUSAPIENTRY glusMouseMoveFunc(GLUSvoid(*glusNewMouseMove)(GLUSint buttons, GLUSint xPos, GLUSint yPos))
{
    glusMouseMove = glusNewMouseMove;
}

GLUSvoid GLUSAPIENTRY glusInitFunc(GLUSboolean(*glusNewInit)(GLUSvoid))
{
    glusInit = glusNewInit;
}

GLUSvoid GLUSAPIENTRY glusReshapeFunc(GLUSvoid(*glusNewReshape)(GLUSint width, GLUSint height))
{
    glusReshape = glusNewReshape;
}

GLUSvoid GLUSAPIENTRY glusUpdateFunc(GLUSboolean(*glusNewUpdate)(GLUSfloat time))
{
    glusUpdate = glusNewUpdate;
}

GLUSvoid GLUSAPIENTRY glusTerminateFunc(GLUSvoid(*glusNewTerminate)(GLUSvoid))
{
    glusTerminate = glusNewTerminate;
}

GLUSvoid GLUSAPIENTRY glusPrepareContext(const GLUSint version)
{
	g_eglContextClientVersion = version;
}

GLUSvoid GLUSAPIENTRY glusPrepareNoResize(const GLUSboolean noResize)
{
	g_noResize = noResize;
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
    if (state == 0)
    {
        if (key == 257)
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

	_glusDestroyNativeWindow();

    g_initdone = GLUS_FALSE;
}

GLUSboolean GLUSAPIENTRY glusCreateWindow(const char* title, const GLUSint width, const GLUSint height, const EGLint* attribList, const GLUSboolean fullscreen)
{
	EGLConfig eglConfig;

	EGLint eglNativeVisualID;

	EGLNativeWindowType eglNativeWindowType;

	if (!glusEGLCreateContext(_glusGetNativeDisplayType(), &g_eglDisplay, &eglConfig, &g_eglContext, attribList, g_eglContextClientVersion))
	{
    	glusLogPrint(GLUS_LOG_ERROR, "Could preinitialize EGL");

    	glusDestroyWindow();

        return GLUS_FALSE;
	}

	if (!glusEGLGetNativeVisualID(g_eglDisplay, eglConfig, &eglNativeVisualID))
	{
    	glusLogPrint(GLUS_LOG_ERROR, "Could not get native visual ID");

    	glusDestroyWindow();

        return GLUS_FALSE;
	}

	eglNativeWindowType = _glusCreateNativeWindowType(title, width, height, fullscreen, g_noResize, eglNativeVisualID);

	if (!eglNativeWindowType)
	{
    	glusLogPrint(GLUS_LOG_ERROR, "Could not create native window");

    	glusDestroyWindow();

        return GLUS_FALSE;
	}

	if (!glusEGLCreateWindowSurfaceMakeCurrent(eglNativeWindowType, &g_eglDisplay, &eglConfig, &g_eglContext, &g_eglSurface))
	{
    	glusLogPrint(GLUS_LOG_ERROR, "Could not post initialize EGL");

    	glusDestroyWindow();

        return GLUS_FALSE;
	}

	_glusGetWindowSize(&g_width, &g_height);

    return GLUS_TRUE; // Success
}

GLUSboolean GLUSAPIENTRY glusRun(GLUSvoid)
{
    // Init Engine
    if (glusInit)
    {
        if (!glusInit())
        {
            glusDestroyWindow(); // Destroy The Window

            return GLUS_FALSE; // Exit The Program
        }
    }

    g_initdone = GLUS_TRUE;

    // Do the first reshape
    if (glusReshape)
    {
        glusReshape(g_width, g_height);
    }

    while (!g_done) // Loop That Runs While done=FALSE
    {
		if (glusUpdate)
		{
			g_done = !glusUpdate(glusGetElapsedTime());
		}

		eglSwapBuffers(g_eglDisplay, g_eglSurface); // Swap Buffers (Double Buffering)

		_glusPollEvents();
    }

    // Terminate Game
    if (glusTerminate)
    {
        glusTerminate();
    }

    // Shutdown
    glusDestroyWindow(); // Destroy The Window

    return GLUS_TRUE; // Exit The Program
}
