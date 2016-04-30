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

#include <android_native_app_glue.h>
#include <time.h>

extern GLUSvoid _glusWindowInternalReshape(GLUSint width, GLUSint height);

extern GLUSvoid _glusWindowInternalClose(GLUSvoid);

extern GLUSvoid _glusWindowInternalMouse(GLUSint button, GLUSint action);

extern GLUSvoid _glusWindowInternalMouseMove(GLUSint x, GLUSint y);

//

static EGLNativeDisplayType g_nativeDisplay = EGL_DEFAULT_DISPLAY;

static EGLNativeWindowType g_nativeWindow = 0;

static GLUSint g_width = -1;

static GLUSint g_height = -1;

//

extern int main(int argc, char* argv[]);

struct android_app* g_app = 0;

static void onConfigurationChanged(ANativeActivity *ac)
{
	// Do nothing, but the callback has to be called.
}

static void onAppCmd(struct android_app* app, int32_t cmd)
{
	if (!app)
	{
		return;
	}

	switch(cmd)
	{
		case APP_CMD_INIT_WINDOW:
			{
				int32_t orientation = AConfiguration_getOrientation(g_app->config);

				g_width = ANativeWindow_getWidth(g_app->window);
				g_height = ANativeWindow_getHeight(g_app->window);

				if (orientation != ACONFIGURATION_ORIENTATION_PORT)
				{
					int32_t  temp = g_width;

					g_width = g_height;
					g_height = temp;
				}

				g_nativeWindow = app->window;
			}
	        break;

	    case APP_CMD_TERM_WINDOW:
	    case APP_CMD_DESTROY:

	    	if (g_nativeWindow)
	    	{
		    	_glusWindowInternalClose();

		    	g_nativeWindow = 0;
	    	}

			g_width = -1;
			g_height = -1;

	    	app->destroyRequested = 1;

	    	break;
	}
}

static int32_t onInputEvent(struct android_app* app, AInputEvent* event)
{
	if (!event)
	{
		return 0;
	}

	int32_t type = AInputEvent_getType(event);

	if (AINPUT_EVENT_TYPE_MOTION == type)
	{
		static float lastX = -1.0f;
		static float lastY = -1.0f;

		static GLUSboolean pressed = GLUS_FALSE;

		//

		float x = AMotionEvent_getX(event, 0);
		float y = AMotionEvent_getY(event, 0);

		if (x != lastX || y != lastY)
		{
			lastX = x;
			lastY = y;

			_glusWindowInternalMouseMove(x, y);
		}

		//

		float pressure = AMotionEvent_getPressure(event, 0);

		if (pressure == 0.0f && pressed)
		{
			pressed = GLUS_FALSE;

			_glusWindowInternalMouse(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
		}
		else if (pressure >= 1.0f && !pressed)
		{
			pressed = GLUS_TRUE;

			_glusWindowInternalMouse(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
		}
	}

	return 0;
}

void android_main(struct android_app* app)
{
	// Make sure glue isn't stripped.
	app_dummy();

	if (!app)
	{
		return;
	}

	g_app = app;

	g_app->onAppCmd = onAppCmd;
	g_app->onInputEvent = onInputEvent;

	g_app->activity->callbacks->onConfigurationChanged = onConfigurationChanged;

	//

	GLUSboolean doInit = GLUS_TRUE;

	while(doInit)
	{
		int ident;
	    int events;
	    struct android_poll_source* androidPollSource;

	    while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&androidPollSource)) >= 0)
	    {
	    	if (androidPollSource != NULL)
	        {
	    		androidPollSource->process(g_app, androidPollSource);
	        }
	    }

	    if (g_app->destroyRequested != 0)
	    {
	    	exit(0);
	    }

	    if (g_nativeWindow != 0)
	    {
	    	doInit = GLUS_FALSE;
	    }
	}

	//

	main(0, 0);

	//

	exit(0);
}

//

GLUSvoid _glusOsPollEvents()
{
	int ident;
    int events;
    struct android_poll_source* androidPollSource;

    while ((ident = ALooper_pollAll(0, NULL, &events, (void**)&androidPollSource)) >= 0)
    {
         if (androidPollSource != NULL)
         {
        	 androidPollSource->process(g_app, androidPollSource);
         }
    }

	int32_t orientation = AConfiguration_getOrientation (g_app->config);

	int32_t width = ANativeWindow_getWidth(g_app->window);
	int32_t height = ANativeWindow_getHeight(g_app->window);

	if (orientation != ACONFIGURATION_ORIENTATION_PORT)
	{
		int32_t temp = width;

		width = height;
		height = temp;
	}

	if (width != g_width || height != g_height)
	{
		g_width = width;
		g_height = height;

		_glusWindowInternalReshape(g_width, g_height);
	}

    if (g_app->destroyRequested != 0 )
    {
    	_glusWindowInternalClose();
    }
}

EGLNativeDisplayType _glusOsGetNativeDisplayType()
{
	return g_nativeDisplay;
}

EGLNativeWindowType _glusOsCreateNativeWindowType(const char* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, EGLint eglNativeVisualID)
{
	glusLogPrint(GLUS_LOG_INFO, "Parameters 'title', 'width', 'height', 'fullscreen' and 'noResize' are not used");
	glusLogPrint(GLUS_LOG_INFO, "Key and mouse events are not supported");

	// Width and height already set.

	return g_nativeWindow;
}

GLUSvoid _glusOsDestroyNativeWindowDisplay()
{
	g_nativeWindow = 0;

	g_nativeDisplay = 0;
}

double _glusOsGetRawTime()
{
	struct timespec currentTime;

	clock_gettime(CLOCK_MONOTONIC, &currentTime);

	return (double)currentTime.tv_sec + (double)currentTime.tv_nsec / 1000000000.0;
}

GLUSvoid _glusOsGetWindowSize(GLUSint* width, GLUSint* height)
{
	if (width)
	{
		*width = g_width;
	}

	if (height)
	{
		*height = g_height;
	}
}
