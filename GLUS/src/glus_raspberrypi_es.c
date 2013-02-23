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

// Terminal input is used

static struct termios g_origTermios;

static void resetTerminalMode()
{
    tcsetattr(0, TCSANOW, &g_origTermios);
}

static void setNonBlockingTerminalMode()
{
    struct termios newTermios;

    tcgetattr(0, &g_origTermios);
    memcpy(&newTermios, &g_origTermios, sizeof(newTermios));

    newTermios.c_lflag &= ~ICANON;
    newTermios.c_lflag &= ~ECHO;
    newTermios.c_lflag &= ~ISIG;
    newTermios.c_cc[VMIN] = 0;
    newTermios.c_cc[VTIME] = 0;

    tcsetattr(0, TCSANOW, &newTermios);
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
	if (!_nativeDisplay)
	{
		_nativeDisplay = vc_dispmanx_display_open(0 /* LCD */);

		if (_nativeDisplay == 0)
		{
			glusLogPrint(GLUS_LOG_ERROR, "Could not open display");

			return EGL_NO_DISPLAY;
		}
	}

	return EGL_DEFAULT_DISPLAY;
}

EGLNativeWindowType _glusCreateNativeWindowType(const char* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, EGLint eglNativeVisualID)
{
	DISPMANX_UPDATE_HANDLE_T dispmanUpdate;
	DISPMANX_ELEMENT_HANDLE_T dispmanElement;
	VC_RECT_T dstRect;
	VC_RECT_T srcRect;
	int success;

	if (_nativeWindowCreated)
	{
		dispmanUpdate = vc_dispmanx_update_start(0);
		vc_dispmanx_element_remove(dispmanUpdate, _nativeWindow.element);
		vc_dispmanx_update_submit_sync(dispmanUpdate);

		_nativeWindowCreated = GLUS_FALSE;
	}
	memset(&_nativeWindow, 0, sizeof(_nativeWindow));

	glusLogPrint(GLUS_LOG_INFO, "Parameters 'title', 'fullscreen' and 'noResize' are not used");
	glusLogPrint(GLUS_LOG_INFO, "Terminal key events are used. Mouse events are not supported");

	//

	dstRect.x = 0;
	dstRect.y = 0;
	dstRect.width = width;
	dstRect.height = height;

	srcRect.x = 0;
	srcRect.y = 0;
	srcRect.width = width << 16;
	srcRect.height = height << 16;

	dispmanUpdate = vc_dispmanx_update_start(0);
	dispmanElement = vc_dispmanx_element_add(dispmanUpdate, _nativeDisplay,
												 0 /*layer*/, &dstRect, 0 /*src*/, &srcRect,
												 DISPMANX_PROTECTION_NONE, 0 /*alpha*/, 0/*clamp*/, 0/*transform*/);
	success = vc_dispmanx_update_submit_sync(dispmanUpdate);
	if (!dispmanElement || !success)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Could not add element");

		return 0;
	}

	_nativeWindow.element = dispmanElement;
	_nativeWindow.width = width;
	_nativeWindow.height = height;

	_width = width;
	_height = height;

	setNonBlockingTerminalMode();

	_nativeWindowCreated = GLUS_TRUE;

	return (EGLNativeWindowType)&_nativeWindow;
}

GLUSvoid _glusDestroyNativeWindow()
{
	DISPMANX_UPDATE_HANDLE_T dispmanUpdate;

    resetTerminalMode();

	if (_nativeWindowCreated)
	{
		dispmanUpdate = vc_dispmanx_update_start(0);
		vc_dispmanx_element_remove(dispmanUpdate, _nativeWindow.element);
		vc_dispmanx_update_submit_sync(dispmanUpdate);

		_nativeWindowCreated = GLUS_FALSE;
	}
	memset(&_nativeWindow, 0, sizeof(_nativeWindow));

	if (_nativeDisplay)
	{
		dispmanUpdate = vc_dispmanx_update_start(0);
		vc_dispmanx_display_close(_nativeDisplay);
		vc_dispmanx_update_submit_sync(dispmanUpdate);

		_nativeDisplay = 0;
	}
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
