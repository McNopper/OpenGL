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

#include <EGL/eglvivante.h>

#include "GL/glus.h"

#include <time.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <linux/input.h>

extern GLUSint glusWindowInternalClose(GLUSvoid);

extern GLUSvoid glusWindowInternalKey(GLUSint key, GLUSint state);

// Map, if possible, to GLFW keys

static int glusOsTranslateKey(int key)
{
	switch (key)
	{
		case KEY_SPACE:
			return GLFW_KEY_SPACE;
		case KEY_APOSTROPHE:
			return GLFW_KEY_APOSTROPHE;
		case KEY_COMMA:
			return GLFW_KEY_COMMA;
		case KEY_MINUS:
			return GLFW_KEY_MINUS;
		case KEY_DOT:
			return GLFW_KEY_PERIOD;
		case KEY_SLASH:
			return GLFW_KEY_SLASH;
		case KEY_0:
			return GLFW_KEY_0;
		case KEY_1:
			return GLFW_KEY_1;
		case KEY_2:
			return GLFW_KEY_2;
		case KEY_3:
			return GLFW_KEY_3;
		case KEY_4:
			return GLFW_KEY_4;
		case KEY_5:
			return GLFW_KEY_5;
		case KEY_6:
			return GLFW_KEY_6;
		case KEY_7:
			return GLFW_KEY_7;
		case KEY_8:
			return GLFW_KEY_8;
		case KEY_9:
			return GLFW_KEY_9;
		case KEY_SEMICOLON:
			return GLFW_KEY_SEMICOLON;
		case KEY_EQUAL:
			return GLFW_KEY_EQUAL;
		case KEY_A:
			return GLFW_KEY_A;
		case KEY_B:
			return GLFW_KEY_B;
		case KEY_C:
			return GLFW_KEY_C;
		case KEY_D:
			return GLFW_KEY_D;
		case KEY_E:
			return GLFW_KEY_E;
		case KEY_F:
			return GLFW_KEY_F;
		case KEY_G:
			return GLFW_KEY_G;
		case KEY_H:
			return GLFW_KEY_H;
		case KEY_I:
			return GLFW_KEY_I;
		case KEY_J:
			return GLFW_KEY_J;
		case KEY_K:
			return GLFW_KEY_K;
		case KEY_L:
			return GLFW_KEY_L;
		case KEY_M:
			return GLFW_KEY_M;
		case KEY_N:
			return GLFW_KEY_N;
		case KEY_O:
			return GLFW_KEY_O;
		case KEY_P:
			return GLFW_KEY_P;
		case KEY_Q:
			return GLFW_KEY_Q;
		case KEY_R:
			return GLFW_KEY_R;
		case KEY_S:
			return GLFW_KEY_S;
		case KEY_T:
			return GLFW_KEY_T;
		case KEY_U:
			return GLFW_KEY_U;
		case KEY_V:
			return GLFW_KEY_V;
		case KEY_W:
			return GLFW_KEY_W;
		case KEY_X:
			return GLFW_KEY_X;
		case KEY_Y:
			return GLFW_KEY_Y;
		case KEY_Z:
			return GLFW_KEY_Z;
		case KEY_LEFTBRACE:
			return GLFW_KEY_LEFT_BRACKET;
		case KEY_BACKSLASH:
			return GLFW_KEY_BACKSLASH;
		case KEY_RIGHTBRACE:
			return GLFW_KEY_RIGHT_BRACKET;
		case KEY_GRAVE:
			return GLFW_KEY_GRAVE_ACCENT;
		case KEY_ESC:
			return GLFW_KEY_ESCAPE;
		case KEY_F1:
			return GLFW_KEY_F1;
		case KEY_F2:
			return GLFW_KEY_F2;
		case KEY_F3:
			return GLFW_KEY_F3;
		case KEY_F4:
			return GLFW_KEY_F4;
		case KEY_F5:
			return GLFW_KEY_F5;
		case KEY_F6:
			return GLFW_KEY_F6;
		case KEY_F7:
			return GLFW_KEY_F7;
		case KEY_F8:
			return GLFW_KEY_F8;
		case KEY_F9:
			return GLFW_KEY_F9;
		case KEY_F10:
			return GLFW_KEY_F10;
		case KEY_F11:
			return GLFW_KEY_F11;
		case KEY_F12:
			return GLFW_KEY_F12;
		case KEY_F13:
			return GLFW_KEY_F13;
		case KEY_F14:
			return GLFW_KEY_F14;
		case KEY_F15:
			return GLFW_KEY_F15;
		case KEY_UP:
			return GLFW_KEY_UP;
		case KEY_DOWN:
			return GLFW_KEY_DOWN;
		case KEY_LEFT:
			return GLFW_KEY_LEFT;
		case KEY_RIGHT:
			return GLFW_KEY_RIGHT;
		case KEY_LEFTSHIFT:
			return GLFW_KEY_LEFT_SHIFT;
		case KEY_RIGHTSHIFT:
			return GLFW_KEY_RIGHT_SHIFT;
		case KEY_LEFTCTRL:
			return GLFW_KEY_LEFT_CONTROL;
		case KEY_RIGHTCTRL:
			return GLFW_KEY_RIGHT_CONTROL;
		case KEY_LEFTALT:
			return GLFW_KEY_LEFT_ALT;
		case KEY_RIGHTALT:
			return GLFW_KEY_RIGHT_ALT;
		case KEY_TAB:
			return GLFW_KEY_TAB;
		case KEY_ENTER:
			return GLFW_KEY_ENTER;
		case KEY_BACKSPACE:
			return GLFW_KEY_BACKSPACE;
		case KEY_INSERT:
			return GLFW_KEY_INSERT;
		case KEY_DELETE:
			return GLFW_KEY_DELETE;
		case KEY_PAGEUP:
			return GLFW_KEY_PAGE_UP;
		case KEY_PAGEDOWN:
			return GLFW_KEY_PAGE_DOWN;
		case KEY_HOME:
			return GLFW_KEY_HOME;
		case KEY_END:
			return GLFW_KEY_END;
		case KEY_KP0:
			return GLFW_KEY_KP_0;
		case KEY_KP1:
			return GLFW_KEY_KP_1;
		case KEY_KP2:
			return GLFW_KEY_KP_2;
		case KEY_KP3:
			return GLFW_KEY_KP_3;
		case KEY_KP4:
			return GLFW_KEY_KP_4;
		case KEY_KP5:
			return GLFW_KEY_KP_5;
		case KEY_KP6:
			return GLFW_KEY_KP_6;
		case KEY_KP7:
			return GLFW_KEY_KP_7;
		case KEY_KP8:
			return GLFW_KEY_KP_8;
		case KEY_KP9:
			return GLFW_KEY_KP_9;
		case KEY_KPSLASH:
			return GLFW_KEY_KP_DIVIDE;
		case KEY_KPASTERISK:
			return GLFW_KEY_KP_MULTIPLY;
		case KEY_KPMINUS:
			return GLFW_KEY_KP_SUBTRACT;
		case KEY_KPPLUS:
			return GLFW_KEY_KP_ADD;
		case KEY_KPDOT:
			return GLFW_KEY_KP_DECIMAL;
		case KEY_KPEQUAL:
			return GLFW_KEY_KP_EQUAL;
		case KEY_KPENTER:
			return GLFW_KEY_KP_ENTER;
		case KEY_NUMLOCK:
			return GLFW_KEY_NUM_LOCK;
		case KEY_CAPSLOCK:
			return GLFW_KEY_CAPS_LOCK;
		case KEY_SCROLLLOCK:
			return GLFW_KEY_SCROLL_LOCK;
		case KEY_PAUSE:
			return GLFW_KEY_PAUSE;
		case KEY_LEFTMETA:
			return GLFW_KEY_LEFT_SUPER;
		case KEY_RIGHTMETA:
			return GLFW_KEY_RIGHT_SUPER;
		case KEY_MENU:
			return GLFW_KEY_MENU;
		default:
			return key;
	}

	return GLFW_KEY_UNKNOWN;
}

//

static EGLNativeDisplayType _nativeDisplay = 0;

static EGLNativeWindowType _nativeWindow = 0;

static GLUSint _width = -1;

static GLUSint _height = -1;

static int _keyFileDescriptor = -1;

GLUSvoid _glusOsPollEvents()
{
	static GLUSboolean LEFT_CTRL = GLUS_FALSE;
	static GLUSboolean RIGHT_CTRL = GLUS_FALSE;

	struct input_event keyEvent;

	if (_keyFileDescriptor >= 0)
	{
		ssize_t numBytes = read(_keyFileDescriptor, &keyEvent, sizeof(struct input_event));

		if (numBytes <= 0 || keyEvent.type != EV_KEY)
		{
			return;
		}

		switch (keyEvent.value)
		{
			case 1:	// Pressed
			{
				// CTRL-C
				if (keyEvent.code == 46 && (LEFT_CTRL || RIGHT_CTRL))
				{
					glusWindowInternalClose();

					return;
				}

				if (keyEvent.code == 29)
				{
					LEFT_CTRL = GLUS_TRUE;
				}
				else if (keyEvent.code == 97)
				{
					RIGHT_CTRL = GLUS_TRUE;
				}

				glusWindowInternalKey(glusOsTranslateKey(keyEvent.code), GLFW_PRESS);
			}
			break;

			case 0: // Released
			{
				if (keyEvent.code == 29)
				{
					LEFT_CTRL = GLUS_FALSE;
				}
				else if (keyEvent.code == 97)
				{
					RIGHT_CTRL = GLUS_FALSE;
				}

				glusWindowInternalKey(glusOsTranslateKey(keyEvent.code), GLFW_RELEASE);
			}
			break;
		}
	}
}

EGLNativeDisplayType _glusOsGetNativeDisplayType()
{
	if (_nativeDisplay != 0)
	{
		return _nativeDisplay;
	}

	_nativeDisplay = fbGetDisplayByIndex(0);

	if (_nativeDisplay == 0)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Could not get native display");

		return 0;
	}

	return _nativeDisplay;
}

EGLNativeWindowType _glusOsCreateNativeWindowType(const char* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, EGLint eglNativeVisualID)
{
	glusLogPrint(GLUS_LOG_INFO, "Parameters 'title', 'fullscreen' and 'noResize' are not used");
	glusLogPrint(GLUS_LOG_INFO, "Key events are mapped to US keyboard");
	glusLogPrint(GLUS_LOG_INFO, "Mouse events are not supported");

	_keyFileDescriptor = open("/dev/input/event1", O_RDONLY | O_NONBLOCK);

	if (_keyFileDescriptor < 0)
	{
		glusLogPrint(GLUS_LOG_WARNING, "Could not open key input device");
	}

	_nativeWindow = fbCreateWindow(_nativeDisplay, 0, 0, width, height);

	if (_nativeWindow == 0)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Could not create native window");

		return 0;
	}

	_width = width;
	_height = height;

	return _nativeWindow;
}

GLUSvoid _glusOsDestroyNativeWindowDisplay()
{
	if (_nativeWindow)
	{
		fbDestroyWindow(_nativeWindow);

		_nativeWindow = 0;
	}

	if (_nativeDisplay)
	{
		fbDestroyDisplay(_nativeDisplay);

		_nativeDisplay = 0;
	}

	if (_keyFileDescriptor >= 0)
	{
		close(_keyFileDescriptor);

		_keyFileDescriptor = -1;
	}
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
		*width = _width;
	}

	if (height)
	{
		*height = _height;
	}
}
