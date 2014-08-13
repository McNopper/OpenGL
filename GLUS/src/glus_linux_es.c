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

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/extensions/Xrandr.h>

#include <time.h>

#include "GL/glus.h"

extern GLUSvoid glusInternalReshape(GLUSint width, GLUSint height);

extern GLUSint glusInternalClose(GLUSvoid);

extern GLUSvoid glusInternalKey(GLUSint key, GLUSint state);

extern GLUSvoid glusInternalMouse(GLUSint button, GLUSint action);

extern GLUSvoid glusInternalMouseWheel(GLUSint pos);

extern GLUSvoid glusInternalMouseMove(GLUSint x, GLUSint y);

static Display* _nativeDisplay = 0;
static Window _nativeWindow = 0;
static Atom _deleteMessage;

static Rotation _oldRotation;
static SizeID _oldSizeID;

//
// Please note: The following lines are taken from GLFW. Some lines are modified for adapting to GLUS.
//

//========================================================================
// GLFW - An OpenGL framework
// Platform:    X11/GLX
// API version: 2.7
// WWW:         http://www.glfw.org/
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

static char _keys[GLFW_KEY_LAST + 1];

GLUSvoid _glusInputMouseClick(GLUSint button, GLUSint action)
{
	if (button >= 0 && button <= GLFW_MOUSE_BUTTON_LAST)
	{
		glusInternalMouse(button, action);
	}
}

GLUSvoid _glusInputKey(GLUSint key, GLUSint action)
{
	if (key < 0 || key > GLFW_KEY_LAST)
	{
		return;
	}

	// Are we trying to release an already released key?
	if (action == GLFW_RELEASE && _keys[key] != GLFW_PRESS)
	{
		return;
	}

	_keys[key] = (char)action;

	glusInternalKey(key, action);
}

GLUSint _glusTranslateKey(int keycode)
{
	KeySym key, key_lc, key_uc;

	// Try secondary keysym, for numeric keypad keys
	// Note: This way we always force "NumLock = ON", which at least
	// enables GLFW users to detect numeric keypad keys
	key = XKeycodeToKeysym(_nativeDisplay, keycode, 1);
	switch (key)
	{
		// Numeric keypad
		case XK_KP_0:
			return GLFW_KEY_KP_0;
		case XK_KP_1:
			return GLFW_KEY_KP_1;
		case XK_KP_2:
			return GLFW_KEY_KP_2;
		case XK_KP_3:
			return GLFW_KEY_KP_3;
		case XK_KP_4:
			return GLFW_KEY_KP_4;
		case XK_KP_5:
			return GLFW_KEY_KP_5;
		case XK_KP_6:
			return GLFW_KEY_KP_6;
		case XK_KP_7:
			return GLFW_KEY_KP_7;
		case XK_KP_8:
			return GLFW_KEY_KP_8;
		case XK_KP_9:
			return GLFW_KEY_KP_9;
		case XK_KP_Separator:
		case XK_KP_Decimal:
			return GLFW_KEY_KP_DECIMAL;
		case XK_KP_Equal:
			return GLFW_KEY_KP_EQUAL;
		case XK_KP_Enter:
			return GLFW_KEY_KP_ENTER;
		default:
		break;
	}

	// Now try pimary keysym
	key = XKeycodeToKeysym(_nativeDisplay, keycode, 0);
	switch (key)
	{
		// Special keys (non character keys)
		case XK_Escape:
			return GLFW_KEY_ESCAPE;
		case XK_Tab:
			return GLFW_KEY_TAB;
		case XK_Shift_L:
			return GLFW_KEY_LEFT_SHIFT;
		case XK_Shift_R:
			return GLFW_KEY_RIGHT_SHIFT;
		case XK_Control_L:
			return GLFW_KEY_LEFT_CONTROL;
		case XK_Control_R:
			return GLFW_KEY_RIGHT_CONTROL;
		case XK_Meta_L:
		case XK_Alt_L:
			return GLFW_KEY_LEFT_ALT;
		case XK_Mode_switch:  // Mapped to Alt_R on many keyboards
		case XK_Meta_R:
		case XK_ISO_Level3_Shift: // AltGr on at least some machines
		case XK_Alt_R:
			return GLFW_KEY_RIGHT_ALT;
		case XK_Super_L:
			return GLFW_KEY_LEFT_SUPER;
		case XK_Super_R:
			return GLFW_KEY_RIGHT_SUPER;
		case XK_Menu:
			return GLFW_KEY_MENU;
		case XK_Num_Lock:
			return GLFW_KEY_NUM_LOCK;
		case XK_Caps_Lock:
			return GLFW_KEY_CAPS_LOCK;
		case XK_Scroll_Lock:
			return GLFW_KEY_SCROLL_LOCK;
		case XK_Pause:
			return GLFW_KEY_PAUSE;
		case XK_KP_Delete:
		case XK_Delete:
			return GLFW_KEY_DELETE;
		case XK_BackSpace:
			return GLFW_KEY_BACKSPACE;
		case XK_Return:
			return GLFW_KEY_ENTER;
		case XK_KP_Home:
		case XK_Home:
			return GLFW_KEY_HOME;
		case XK_KP_End:
		case XK_End:
			return GLFW_KEY_END;
		case XK_KP_Page_Up:
		case XK_Page_Up:
			return GLFW_KEY_PAGE_UP;
		case XK_KP_Page_Down:
		case XK_Page_Down:
			return GLFW_KEY_PAGE_DOWN;
		case XK_KP_Insert:
		case XK_Insert:
			return GLFW_KEY_INSERT;
		case XK_KP_Left:
		case XK_Left:
			return GLFW_KEY_LEFT;
		case XK_KP_Right:
		case XK_Right:
			return GLFW_KEY_RIGHT;
		case XK_KP_Down:
		case XK_Down:
			return GLFW_KEY_DOWN;
		case XK_KP_Up:
		case XK_Up:
			return GLFW_KEY_UP;
		case XK_F1:
			return GLFW_KEY_F1;
		case XK_F2:
			return GLFW_KEY_F2;
		case XK_F3:
			return GLFW_KEY_F3;
		case XK_F4:
			return GLFW_KEY_F4;
		case XK_F5:
			return GLFW_KEY_F5;
		case XK_F6:
			return GLFW_KEY_F6;
		case XK_F7:
			return GLFW_KEY_F7;
		case XK_F8:
			return GLFW_KEY_F8;
		case XK_F9:
			return GLFW_KEY_F9;
		case XK_F10:
			return GLFW_KEY_F10;
		case XK_F11:
			return GLFW_KEY_F11;
		case XK_F12:
			return GLFW_KEY_F12;
		case XK_F13:
			return GLFW_KEY_F13;
		case XK_F14:
			return GLFW_KEY_F14;
		case XK_F15:
			return GLFW_KEY_F15;
		case XK_F16:
			return GLFW_KEY_F16;
		case XK_F17:
			return GLFW_KEY_F17;
		case XK_F18:
			return GLFW_KEY_F18;
		case XK_F19:
			return GLFW_KEY_F19;
		case XK_F20:
			return GLFW_KEY_F20;
		case XK_F21:
			return GLFW_KEY_F21;
		case XK_F22:
			return GLFW_KEY_F22;
		case XK_F23:
			return GLFW_KEY_F23;
		case XK_F24:
			return GLFW_KEY_F24;
		case XK_F25:
			return GLFW_KEY_F25;

			// Numeric keypad (should have been detected in secondary keysym!)
		case XK_KP_Divide:
			return GLFW_KEY_KP_DIVIDE;
		case XK_KP_Multiply:
			return GLFW_KEY_KP_MULTIPLY;
		case XK_KP_Subtract:
			return GLFW_KEY_KP_SUBTRACT;
		case XK_KP_Add:
			return GLFW_KEY_KP_ADD;
		case XK_KP_Equal:
			return GLFW_KEY_KP_EQUAL;
		case XK_KP_Enter:
			return GLFW_KEY_KP_ENTER;

			// The rest (should be printable keys)
		default:
			// Make uppercase
			XConvertCase(key, &key_lc, &key_uc);
			key = key_uc;

			// Valid ISO 8859-1 character?
			if ((key >= 32 && key <= 126) || (key >= 160 && key <= 255))
			{
				return (int)key;
			}
			return GLFW_KEY_UNKNOWN;
	}
}

//
// Now, own code
//

static GLUSint _width = -1;

static GLUSint _height = -1;

static GLUSint _wheelPos = 0;

static GLUSboolean _fullscreen = GLUS_FALSE;

GLUSvoid _glusProcessWindow(const XEvent* msg)
{
	if (!msg)
	{
		return;
	}

	switch (msg->type)
	{
		case KeyPress:
			_glusInputKey(_glusTranslateKey(msg->xkey.keycode), GLFW_PRESS);
		break;
		case KeyRelease:
			_glusInputKey(_glusTranslateKey(msg->xkey.keycode), GLFW_RELEASE);
		break;
		case ButtonPress:
			switch (msg->xbutton.button)
			{
				case 1:
					_glusInputMouseClick(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
				break;
				case 2:
					_glusInputMouseClick(GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS);
				break;
				case 3:
					_glusInputMouseClick(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);
				break;
			}
		break;
		case ButtonRelease:
			switch (msg->xbutton.button)
			{
				case 1:
					_glusInputMouseClick(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
				break;
				case 2:
					_glusInputMouseClick(GLFW_MOUSE_BUTTON_MIDDLE, GLFW_RELEASE);
				break;
				case 3:
					_glusInputMouseClick(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE);
				break;
				case 4:
					_wheelPos += 1;

					glusInternalMouseWheel(_wheelPos);
				break;
				case 5:
					_wheelPos -= 1;

					glusInternalMouseWheel(_wheelPos);
				break;
			}
		break;
		case MotionNotify:
			glusInternalMouseMove(msg->xmotion.x, msg->xmotion.y);
		break;
		case ConfigureNotify:

			if (msg->xconfigure.width != _width || msg->xconfigure.height != _height)
			{
				_width = msg->xconfigure.width;
				_height = msg->xconfigure.height;

				glusInternalReshape(_width, _height);
			}
		break;
	}
}

GLUSvoid _glusPollEvents()
{
	XEvent msg;

	while (XPending(_nativeDisplay) > 0)
	{
		XNextEvent(_nativeDisplay, &msg);

		if (msg.type == ClientMessage && msg.xclient.data.l[0] == _deleteMessage)
		{
			glusInternalClose();
		}
		else
		{
			_glusProcessWindow(&msg);
		}
	}
}

EGLNativeDisplayType _glusGetNativeDisplayType()
{
	if (!_nativeDisplay)
	{
		_nativeDisplay = XOpenDisplay(0);
	}

	return (EGLNativeDisplayType)_nativeDisplay;
}

EGLNativeWindowType _glusCreateNativeWindowType(const char* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, const GLUSint nativeVisualID)
{
	long defaultScreen;
	XVisualInfo visualInfoTemplate;
	int numberOfVisuals;
	XVisualInfo* visualInfo;
	Colormap colormap;
	XSetWindowAttributes windowAttributes;

	defaultScreen = DefaultScreen(_nativeDisplay);

	if (fullscreen)
	{
		XRRScreenConfiguration* screenConfiguration;
		int numberSupportedModes;
		XRRScreenSize *sizeList;
		int i;

		screenConfiguration = XRRGetScreenInfo(_nativeDisplay, RootWindow(_nativeDisplay, defaultScreen));

		if (!screenConfiguration)
		{
			glusLogPrint(GLUS_LOG_ERROR, "Could not get screen info");

			return 0;
		}

		sizeList = XRRConfigSizes(screenConfiguration, &numberSupportedModes);

		// Find the best matching mode
		for (i = 0; i < numberSupportedModes; i++)
		{
			if (sizeList[i].width == width && sizeList[i].height == height)
			{
				break;
			}
		}

		if (i == numberSupportedModes)
		{
			XRRFreeScreenConfigInfo(screenConfiguration);

			glusLogPrint(GLUS_LOG_ERROR, "No matching display resolution found: ", width, height);

			return 0;
		}

		// Store current configuration.
		_oldSizeID = XRRConfigCurrentConfiguration(screenConfiguration, &_oldRotation);

		XRRSetScreenConfig(_nativeDisplay, screenConfiguration, RootWindow(_nativeDisplay, defaultScreen), i, RR_Rotate_0, CurrentTime);

		XRRFreeScreenConfigInfo(screenConfiguration);
	}

	visualInfoTemplate.visualid = nativeVisualID;
	visualInfo = XGetVisualInfo(_nativeDisplay, VisualIDMask, &visualInfoTemplate, &numberOfVisuals);

	if (!visualInfo)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Could not get visual info");

		return 0;
	}

	colormap = XCreateColormap(_nativeDisplay, RootWindow(_nativeDisplay, defaultScreen), visualInfo->visual, AllocNone);

	windowAttributes.colormap = colormap;
	windowAttributes.background_pixel = 0xFFFFFFFF;
	windowAttributes.border_pixel = 0;
	windowAttributes.event_mask = KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | EnterWindowMask | LeaveWindowMask | PointerMotionMask | StructureNotifyMask | ExposureMask;

	_nativeWindow = XCreateWindow(_nativeDisplay, RootWindow(_nativeDisplay, defaultScreen), 0, 0, width, height, 0, visualInfo->depth, InputOutput, visualInfo->visual, CWBackPixel | CWBorderPixel | CWEventMask | CWColormap, &windowAttributes);

	XSetStandardProperties(_nativeDisplay, _nativeWindow, title, "", 0, 0, 0, 0);
	XMapWindow(_nativeDisplay, _nativeWindow);
	XSetWMColormapWindows(_nativeDisplay, _nativeWindow, &_nativeWindow, 1);
	XFlush(_nativeDisplay);

	_deleteMessage = XInternAtom(_nativeDisplay, "WM_DELETE_WINDOW", False);
	XSetWMProtocols(_nativeDisplay, _nativeWindow, &_deleteMessage, 1);

	if (noResize)
	{
		XSizeHints* sizeHints = XAllocSizeHints();
		sizeHints->flags |= (PMinSize | PMaxSize);
		sizeHints->min_width = sizeHints->max_width = width;
		sizeHints->min_height = sizeHints->max_height = height;
		XSetWMNormalHints(_nativeDisplay, _nativeWindow, sizeHints);
		XFree(sizeHints);
	}

	if (fullscreen)
	{
		XEvent xev;

		Atom stateFullscreenMessage = XInternAtom(_nativeDisplay, "_NET_WM_STATE_FULLSCREEN", False);
		Atom stateMessage = XInternAtom(_nativeDisplay, "_NET_WM_STATE", False);

		memset(&xev, 0, sizeof(xev));
		xev.type = ClientMessage;
		xev.xclient.window = _nativeWindow;
		xev.xclient.message_type = stateMessage;
		xev.xclient.format = 32;
		xev.xclient.data.l[0] = 1;
		xev.xclient.data.l[1] = stateFullscreenMessage;
		xev.xclient.data.l[2] = 0;

		XSendEvent(_nativeDisplay, DefaultRootWindow(_nativeDisplay), False, SubstructureRedirectMask | SubstructureNotifyMask, &xev);
	}

	_fullscreen = fullscreen;

	_width = width;
	_height = height;

	return _nativeWindow;
}

GLUSvoid _glusDestroyNativeWindowDisplay()
{
	if (_nativeDisplay && _fullscreen)
	{
		long defaultScreen;
		XRRScreenConfiguration* screenConfiguration;

		defaultScreen = DefaultScreen(_nativeDisplay);
		screenConfiguration = XRRGetScreenInfo(_nativeDisplay, RootWindow(_nativeDisplay, defaultScreen));

		if (screenConfiguration)
		{
			XRRSetScreenConfig(_nativeDisplay, screenConfiguration, RootWindow(_nativeDisplay, defaultScreen), _oldSizeID, _oldRotation, CurrentTime);

			XRRFreeScreenConfigInfo(screenConfiguration);
		}

		_fullscreen = GLUS_FALSE;
	}

	if (_nativeDisplay && _nativeWindow)
	{
		XDestroyWindow(_nativeDisplay, _nativeWindow);

		_nativeWindow = 0;
	}

	if (_nativeDisplay)
	{
		XCloseDisplay(_nativeDisplay);

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
