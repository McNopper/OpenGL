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

static GLUSint g_major = 1;
static GLUSint g_minor = 0;
static GLUSint g_flags = 0;

static GLUSint g_numberSamples = 0;
static GLUSboolean g_noResize = GLUS_FALSE;

static GLUSboolean g_initdone = GLUS_FALSE;
static GLUSboolean g_done = GLUS_FALSE;
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

GLUSvoid GLUSAPIENTRY glusPrepareContext(const GLUSint major, const GLUSint minor, const GLUSint flags)
{
    g_major = major;
    g_minor = minor;
    g_flags = flags;

	if (g_major < 1)
	{
		g_major = 1;
	}
	if (g_minor < 0)
	{
		g_minor = 0;
	}
}

GLUSvoid GLUSAPIENTRY glusPrepareMSAA(const GLUSint numberSamples)
{
    g_numberSamples = numberSamples;

	if (g_numberSamples < 0)
	{
		g_numberSamples = 0;
	}
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

    measuredTime = (GLUSfloat) glfwGetTime();

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

static GLUSvoid glusInternalReshape(GLUSint width, GLUSint height)
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

static GLUSint glusInternalClose(void)
{
    g_done = GLUS_TRUE;

    return 0;
}

static GLUSvoid glusInternalKey(GLUSint key, GLUSint state)
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

static GLUSvoid glusInternalMouse(GLUSint button, GLUSint action)
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

static GLUSvoid glusInternalMouseWheel(GLUSint pos)
{
    if (glusMouseWheel)
    {
        glusMouseWheel(g_buttons, pos, g_mouseX, g_mouseY);
    }
}

static GLUSvoid glusInternalMouseMove(GLUSint x, GLUSint y)
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
    glfwTerminate();

    g_initdone = GLUS_FALSE;
}

GLUSboolean GLUSAPIENTRY glusCreateWindow(const char* title, const GLUSint width, const GLUSint height, const GLUSint depthBits, const GLUSint stencilBits, const GLUSboolean fullscreen)
{
    GLUSenum err;

    if (!glfwInit())
    {
    	glusLogPrint(GLUS_LOG_ERROR, "GLFW could not be initialized");

        return GLUS_FALSE;
    }

    glfwCloseWindow();

    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MAJOR, g_major);
    glfwOpenWindowHint(GLFW_OPENGL_VERSION_MINOR, g_minor);
    glfwOpenWindowHint(GLFW_OPENGL_FORWARD_COMPAT, g_flags & GLUS_FORWARD_COMPATIBLE_BIT);
    glfwOpenWindowHint(GLFW_OPENGL_PROFILE, (g_flags & GLUS_FORWARD_COMPATIBLE_BIT) ? GLFW_OPENGL_CORE_PROFILE : GLFW_OPENGL_COMPAT_PROFILE);
    glfwOpenWindowHint(GLFW_FSAA_SAMPLES, g_numberSamples);
    glfwOpenWindowHint(GLFW_WINDOW_NO_RESIZE, g_noResize);

    // If we do not call this function in advance, glfwOpenWindow crashes in Debug under Windows
    glfwSetWindowTitle("");
    if (!glfwOpenWindow(width, height, 8, 8, 8, 8, depthBits, stencilBits, fullscreen ? GLFW_FULLSCREEN : GLFW_WINDOW))
    {
    	glfwTerminate();

    	glusLogPrint(GLUS_LOG_ERROR, "GLFW window could not be opened");

        return GLUS_FALSE;
    }
    glfwSetWindowTitle(title);

    glewExperimental = GLUS_TRUE;

    err = glewInit();

    if (GLUS_OK != err)
    {
        glusDestroyWindow();

    	glusLogPrint(GLUS_LOG_ERROR, "GLEW could not be initialized: %x", err);

        return GLUS_FALSE;
    }

    if (!glusIsSupported(g_major, g_minor))
    {
        glusDestroyWindow();

        glusLogPrint(GLUS_LOG_ERROR, "OpenGL %u.%u not supported", g_major, g_minor);

        return GLUS_FALSE;
    }

    glfwSetWindowSizeCallback(glusInternalReshape);
    glfwSetWindowCloseCallback(glusInternalClose);
    glfwSetKeyCallback(glusInternalKey);
    glfwSetMouseButtonCallback(glusInternalMouse);
    glfwSetMouseWheelCallback(glusInternalMouseWheel);
    glfwSetMousePosCallback(glusInternalMouseMove);

    glfwGetWindowSize(&g_width, &g_height);

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

		glfwSwapBuffers(); // Swap Buffers (Double Buffering)
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
