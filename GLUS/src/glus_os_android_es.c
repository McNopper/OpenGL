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
				int32_t orientation = AConfiguration_getOrientation (g_app->config);

				int32_t width;
				int32_t height;

				if (orientation == ACONFIGURATION_ORIENTATION_PORT)
				{
					width = ANativeWindow_getWidth(g_app->window);
					height = ANativeWindow_getHeight(g_app->window);
				}
				else
				{
					height = ANativeWindow_getWidth(g_app->window);
					width = ANativeWindow_getHeight(g_app->window);
				}

				if (width != g_width || height != g_height)
				{
					g_width = width;
					g_height = height;
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

    while ((ident = ALooper_pollAll( 0, NULL, &events, (void**)&androidPollSource)) >= 0)
    {
         if (androidPollSource != NULL)
         {
        	 androidPollSource->process(g_app, androidPollSource);
         }
    }

	int32_t orientation = AConfiguration_getOrientation (g_app->config);

	int32_t width;
	int32_t height;

	if (orientation == ACONFIGURATION_ORIENTATION_PORT)
	{
		width = ANativeWindow_getWidth(g_app->window);
		height = ANativeWindow_getHeight(g_app->window);
	}
	else
	{
		height = ANativeWindow_getWidth(g_app->window);
		width = ANativeWindow_getHeight(g_app->window);
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
