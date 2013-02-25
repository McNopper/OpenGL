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

#include <time.h>

#include <termios.h>

#include <bcm_host.h>

extern GLUSint glusInternalClose(GLUSvoid);

extern GLUSvoid glusInternalKey(GLUSint key, GLUSint state);

// Display resolution changing

static pthread_mutex_t _mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t _cond = PTHREAD_COND_INITIALIZER;
static GLUSboolean _resizeDone = GLUS_FALSE;

static void resizeDone(void *callback_data, uint32_t reason, uint32_t param1, uint32_t param2)
{
	pthread_mutex_lock(&_mutex);
	_resizeDone = GLUS_TRUE;
	pthread_cond_signal(&_cond);
	pthread_mutex_unlock(&_mutex);
}

static void waitResizeDone()
{
	GLUSboolean doBreak = GLUS_FALSE;

	struct timespec breakTime;
	clock_gettime(CLOCK_REALTIME, &breakTime);

	breakTime.tv_sec += 8;

	while(!doBreak)
	{
		pthread_mutex_lock(&_mutex);
		if (pthread_cond_timedwait(&_cond, &_mutex, &breakTime) == 0)
		{
			doBreak = _resizeDone;
		}
		else
		{
			doBreak = GLUS_TRUE;
		}
		pthread_mutex_unlock(&_mutex);
	}
}

// Terminal input is used

static struct termios g_originalTermios;

static GLUSboolean g_hasOriginalTermios = GLUS_FALSE;

static void resetTerminalMode()
{
    if (g_hasOriginalTermios)
    {
    	tcsetattr(0, TCSANOW, &g_originalTermios);

    	g_hasOriginalTermios = GLUS_FALSE;
    }
}

static void setNonBlockingTerminalMode()
{
    struct termios newTermios;

    if (!g_hasOriginalTermios)
    {
		tcgetattr(0, &g_originalTermios);
		memcpy(&newTermios, &g_originalTermios, sizeof(newTermios));

		newTermios.c_lflag &= ~ICANON;
		newTermios.c_lflag &= ~ECHO;
		newTermios.c_lflag &= ~ISIG;
		newTermios.c_cc[VMIN] = 0;
		newTermios.c_cc[VTIME] = 0;

		tcsetattr(0, TCSANOW, &newTermios);

		g_hasOriginalTermios = GLUS_TRUE;
    }
}

// Map, if possible, to GLFW keys

static int translateKey(int key)
{
	int keys[4];

	switch (key)
	{
		case 9:
			// Tab key
			return GLFW_KEY_SPECIAL + 37;
		case 10:
			// Return key
			return GLFW_KEY_SPECIAL + 38;
		case 127:
			// Backspace key
			return GLFW_KEY_SPECIAL + 39;
	}

	// Normal key
	if (key != 27)
	{
		return key;
	}

	keys[0] = getchar();

	// Escape key
	if (keys[0] == -1)
	{
		return GLFW_KEY_SPECIAL + 1;
	}

	keys[1] = getchar();
	keys[2] = getchar();
	keys[3] = getchar();

	//glusLogPrint(GLUS_LOG_SEVERE, "Keys %d - %d %d %d %d\n", key, keys[0], keys[1], keys[2], keys[3]);

	if (keys[0] == 91)
	{
		switch (keys[1])
		{
			case 49:
				// Insert key
				return GLFW_KEY_SPECIAL + 44;
			case 50:
				// Del key
				return GLFW_KEY_SPECIAL + 40;
			case 51:
				// Page up key
				return GLFW_KEY_SPECIAL + 41;
			case 52:
				// Page down key
				return GLFW_KEY_SPECIAL + 45;
			case 53:
				// Home/Pos1 key
				return GLFW_KEY_SPECIAL + 42;
			case 54:
				// End key
				return GLFW_KEY_SPECIAL + 43;
			case 65:
			case 66:
				// Cursor keys
				return GLFW_KEY_SPECIAL + 27 + keys[1] - 65;
			case 67:
			case 68:
				// Cursor keys
				return GLFW_KEY_SPECIAL + 29 + 68 - keys[1];
			case 80:
				// Pause key
				return GLFW_KEY_SPECIAL + 66;
		}
	}

	if (keys[1] == 91)
	{
		if (keys[2] >= 65 && keys[2] <= 69)
		{
			// Function keys
			return GLFW_KEY_SPECIAL + 2 + keys[2] - 65;
		}
	}
	else if (keys[1] == 49)
	{
		if (keys[2] == 70)
		{
			// Function keys
			return GLFW_KEY_SPECIAL + 2 + keys[2] - 65;
		}
		else if (keys[2] >= 55 && keys[2] <= 57)
		{
			// Function keys
			return GLFW_KEY_SPECIAL + 2 + keys[2] - 55 + 5;
		}
	}
	else if (keys[1] == 50)
	{
		if (keys[2] >= 48 && keys[2] <= 49)
		{
			// Function keys
			return GLFW_KEY_SPECIAL + 2 + keys[2] - 48 + 8;
		}
		else if (keys[2] >= 51 && keys[2] <= 54)
		{
			// Function keys
			return GLFW_KEY_SPECIAL + 2 + keys[2] - 51 + 10;
		}
		else if (keys[2] >= 56 && keys[2] <= 57)
		{
			// Function keys
			return GLFW_KEY_SPECIAL + 2 + keys[2] - 56 + 14;
		}
	}
	else if (keys[1] == 51)
	{
		if (keys[2] >= 49 && keys[2] <= 52)
		{
			// Function keys
			return GLFW_KEY_SPECIAL + 2 + keys[2] - 49 + 16;
		}
	}

	return GLFW_KEY_UNKNOWN;
}

//

static GLUSboolean _nativeWindowCreated = GLUS_FALSE;

static DISPMANX_DISPLAY_HANDLE_T _nativeDisplay = 0;

static EGL_DISPMANX_WINDOW_T _nativeWindow;

static GLUSint _width = -1;

static GLUSint _height = -1;

static GLUSboolean _fullscreen = GLUS_FALSE;

GLUSvoid _glusPollEvents()
{
    static int lastKey = 0;
    static GLUSboolean keyPressed = GLUS_FALSE;

    int currentKey = getchar();

    if (currentKey != EOF)
    {
		lastKey = currentKey;

		// CTRL-C
		if (lastKey == 3)
		{
			lastKey = 0;

			glusInternalClose();

			return;
		}

		lastKey = translateKey(lastKey);

		keyPressed = GLUS_TRUE;

		glusInternalKey(lastKey, keyPressed);
    }
	else if (keyPressed)
	{
		keyPressed = GLUS_FALSE;

		glusInternalKey(lastKey, keyPressed);

		lastKey = 0;
    }
}

EGLNativeDisplayType _glusGetNativeDisplayType()
{
	return EGL_DEFAULT_DISPLAY;
}

EGLNativeWindowType _glusCreateNativeWindowType(const char* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, EGLint eglNativeVisualID)
{
	DISPMANX_UPDATE_HANDLE_T dispmanUpdate;
	DISPMANX_ELEMENT_HANDLE_T dispmanElement;
	VC_RECT_T dstRect;
	VC_RECT_T srcRect;
	VC_DISPMANX_ALPHA_T dispmanAlpha;
	int32_t success;
	int32_t windowWidth;
	int32_t windowHeight;

	glusLogPrint(GLUS_LOG_INFO, "Parameters 'title' and 'noResize' are not used");
	glusLogPrint(GLUS_LOG_INFO, "Terminal key events are used. Mouse events are not supported");

	// Initialize graphics system

	bcm_host_init();

	// Set fullscreen, if wanted

	if (fullscreen)
	{
		const uint32_t MAX_SUPPORTED_MODES = 128;
		HDMI_RES_GROUP_T group = HDMI_RES_GROUP_DMT;
		TV_SUPPORTED_MODE_NEW_T supportedModes[MAX_SUPPORTED_MODES];
		int32_t i, numberSupportedModes;

		numberSupportedModes = vc_tv_hdmi_get_supported_modes_new(group, supportedModes, MAX_SUPPORTED_MODES, 0, 0);

		for (i = 0; i < numberSupportedModes; i++)
		{
			if (supportedModes[i].width == (uint32_t)width && supportedModes[i].height == (uint32_t)height && supportedModes[i].frame_rate >= 60)
			{
				break;
			}
		}

		if (i == numberSupportedModes)
		{
			glusLogPrint(GLUS_LOG_ERROR, "No matching display resolution found: ", width, height);

			return EGL_NO_SURFACE;
		}

		vc_tv_register_callback(resizeDone, 0);

		if (vc_tv_hdmi_power_on_explicit_new(group, supportedModes[i].group, supportedModes[i].code) != 0)
		{
			vc_tv_unregister_callback(resizeDone);

			glusLogPrint(GLUS_LOG_ERROR, "Could not switch to full screen: ", width, height);

			return EGL_NO_SURFACE;
		}

		waitResizeDone();

		vc_tv_unregister_callback(resizeDone);

		windowWidth = width;
		windowHeight = height;

		_fullscreen = GLUS_TRUE;
	}
	else
	{
		windowWidth = width;
		windowHeight = height;
	}

	//

	_nativeDisplay = vc_dispmanx_display_open(0 /* LCD */);

	if (!_nativeDisplay)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Could not open display");

		return EGL_NO_SURFACE;
	}

	//

	dstRect.x = 0;
	dstRect.y = 0;
	dstRect.width = windowWidth;
	dstRect.height = windowHeight;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.width = windowWidth << 16;
	srcRect.height = windowHeight << 16;

	dispmanAlpha.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
	dispmanAlpha.mask = 0xFFFFFFFF;
	dispmanAlpha.opacity = 255;

	dispmanUpdate = vc_dispmanx_update_start(0);

	dispmanElement = vc_dispmanx_element_add(dispmanUpdate, _nativeDisplay,
												 0 /*layer*/, &dstRect, 0 /*src*/, &srcRect,
												 DISPMANX_PROTECTION_NONE, &dispmanAlpha, 0/*clamp*/, 0/*transform*/);

	success = vc_dispmanx_update_submit_sync(dispmanUpdate);

	if (!dispmanElement || success < 0)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Could not add element");

		return EGL_NO_SURFACE;
	}

	_width = windowWidth;
	_height = windowHeight;

	_nativeWindow.element = dispmanElement;
	_nativeWindow.width = windowWidth;
	_nativeWindow.height = windowHeight;

	_nativeWindowCreated = GLUS_TRUE;

	setNonBlockingTerminalMode();

	return (EGLNativeWindowType)&_nativeWindow;
}

GLUSvoid _glusDestroyNativeWindow()
{
    resetTerminalMode();

	if (_nativeWindowCreated)
	{
		DISPMANX_UPDATE_HANDLE_T dispmanUpdate;

		dispmanUpdate = vc_dispmanx_update_start(0);
		vc_dispmanx_element_remove(dispmanUpdate, _nativeWindow.element);
		vc_dispmanx_update_submit_sync(dispmanUpdate);

		_nativeWindowCreated = GLUS_FALSE;
	}
	memset(&_nativeWindow, 0, sizeof(_nativeWindow));

	if (_nativeDisplay)
	{
		vc_dispmanx_display_close(_nativeDisplay);

		_nativeDisplay = 0;
	}

	if (_fullscreen)
	{
		vc_tv_register_callback(resizeDone, 0);

		if (vc_tv_hdmi_power_on_preferred() == 0)
		{
			waitResizeDone();
		}

		vc_tv_unregister_callback(resizeDone);

		_fullscreen = GLUS_FALSE;
	}

	bcm_host_deinit();
}

double _glusGetRawTime()
{
	struct timespec currentTime;

	clock_gettime(CLOCK_MONOTONIC, &currentTime);

    return (double)currentTime.tv_sec + (double)currentTime.tv_nsec / 1000000000.0;
}

GLUSvoid _glusGetWindowSize(GLUSint* width, GLUSint* height)
{
	if (width)
	{
		*width = _width;
	}

	if (height)
	{
		*height = _height;
	}
}
