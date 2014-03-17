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

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <mmsystem.h>

#include "GL/glus.h"

extern GLUSvoid glusInternalReshape(GLUSint width, GLUSint height);

extern GLUSint glusInternalClose(GLUSvoid);

extern GLUSvoid glusInternalKey(GLUSint key, GLUSint state);

extern GLUSvoid glusInternalMouse(GLUSint button, GLUSint action);

extern GLUSvoid glusInternalMouseWheel(GLUSint pos);

extern GLUSvoid glusInternalMouseMove(GLUSint x, GLUSint y);

//
// Please note: The following lines are taken from GLFW. Some lines are modified for adapting to GLUS.
//

//========================================================================
// GLFW - An OpenGL framework
// Platform:    Win32/WGL
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

GLUSint _glusTranslateKey(WPARAM wParam, LPARAM lParam)
{
	MSG next_msg;
	DWORD msg_time;
	DWORD scan_code;

	// Check for numeric keypad keys
	// Note: This way we always force "NumLock = ON", which at least
	// enables GLFW users to detect numeric keypad keys
	int hiFlags = HIWORD(lParam);

	if (!(hiFlags & 0x100))
	{
		switch (MapVirtualKey(hiFlags & 0xFF, 1))
		{
			case VK_INSERT:
				return GLFW_KEY_KP_0;
			case VK_END:
				return GLFW_KEY_KP_1;
			case VK_DOWN:
				return GLFW_KEY_KP_2;
			case VK_NEXT:
				return GLFW_KEY_KP_3;
			case VK_LEFT:
				return GLFW_KEY_KP_4;
			case VK_CLEAR:
				return GLFW_KEY_KP_5;
			case VK_RIGHT:
				return GLFW_KEY_KP_6;
			case VK_HOME:
				return GLFW_KEY_KP_7;
			case VK_UP:
				return GLFW_KEY_KP_8;
			case VK_PRIOR:
				return GLFW_KEY_KP_9;
			case VK_DIVIDE:
				return GLFW_KEY_KP_DIVIDE;
			case VK_MULTIPLY:
				return GLFW_KEY_KP_MULTIPLY;
			case VK_SUBTRACT:
				return GLFW_KEY_KP_SUBTRACT;
			case VK_ADD:
				return GLFW_KEY_KP_ADD;
			case VK_DELETE:
				return GLFW_KEY_KP_DECIMAL;
		}
	}

	// Check which key was pressed or released
	switch (wParam)
	{
		// The SHIFT keys require special handling
		case VK_SHIFT:
		{
			// Compare scan code for this key with that of VK_RSHIFT in
			// order to determine which shift key was pressed (left or
			// right)
			scan_code = MapVirtualKey(VK_RSHIFT, 0);
			if (((lParam & 0x01ff0000) >> 16) == scan_code)
			{
				return GLFW_KEY_RIGHT_SHIFT;
			}

			return GLFW_KEY_LEFT_SHIFT;
		}

			// The CTRL keys require special handling
		case VK_CONTROL:
		{
			// Is this an extended key (i.e. right key)?
			if (lParam & 0x01000000)
			{
				return GLFW_KEY_RIGHT_CONTROL;
			}

			// Here is a trick: "Alt Gr" sends LCTRL, then RALT. We only
			// want the RALT message, so we try to see if the next message
			// is a RALT message. In that case, this is a false LCTRL!
			msg_time = GetMessageTime();
			if (PeekMessage(&next_msg, NULL, 0, 0, PM_NOREMOVE))
			{
				if (next_msg.message == WM_KEYDOWN || next_msg.message == WM_SYSKEYDOWN)
				{
					if (next_msg.wParam == VK_MENU && (next_msg.lParam & 0x01000000) && next_msg.time == msg_time)
					{
						// Next message is a RALT down message, which
						// means that this is NOT a proper LCTRL message!
						return GLFW_KEY_UNKNOWN;
					}
				}
			}

			return GLFW_KEY_LEFT_CONTROL;
		}

			// The ALT keys require special handling
		case VK_MENU:
		{
			// Is this an extended key (i.e. right key)?
			if (lParam & 0x01000000)
			{
				return GLFW_KEY_RIGHT_ALT;
			}

			return GLFW_KEY_LEFT_ALT;
		}

			// The ENTER keys require special handling
		case VK_RETURN:
		{
			// Is this an extended key (i.e. right key)?
			if (lParam & 0x01000000)
			{
				return GLFW_KEY_KP_ENTER;
			}

			return GLFW_KEY_ENTER;
		}

			// Special keys (non character keys)
		case VK_ESCAPE:
			return GLFW_KEY_ESCAPE;
		case VK_TAB:
			return GLFW_KEY_TAB;
		case VK_BACK:
			return GLFW_KEY_BACKSPACE;
		case VK_HOME:
			return GLFW_KEY_HOME;
		case VK_END:
			return GLFW_KEY_END;
		case VK_PRIOR:
			return GLFW_KEY_PAGE_UP;
		case VK_NEXT:
			return GLFW_KEY_PAGE_DOWN;
		case VK_INSERT:
			return GLFW_KEY_INSERT;
		case VK_DELETE:
			return GLFW_KEY_DELETE;
		case VK_LEFT:
			return GLFW_KEY_LEFT;
		case VK_UP:
			return GLFW_KEY_UP;
		case VK_RIGHT:
			return GLFW_KEY_RIGHT;
		case VK_DOWN:
			return GLFW_KEY_DOWN;
		case VK_F1:
			return GLFW_KEY_F1;
		case VK_F2:
			return GLFW_KEY_F2;
		case VK_F3:
			return GLFW_KEY_F3;
		case VK_F4:
			return GLFW_KEY_F4;
		case VK_F5:
			return GLFW_KEY_F5;
		case VK_F6:
			return GLFW_KEY_F6;
		case VK_F7:
			return GLFW_KEY_F7;
		case VK_F8:
			return GLFW_KEY_F8;
		case VK_F9:
			return GLFW_KEY_F9;
		case VK_F10:
			return GLFW_KEY_F10;
		case VK_F11:
			return GLFW_KEY_F11;
		case VK_F12:
			return GLFW_KEY_F12;
		case VK_F13:
			return GLFW_KEY_F13;
		case VK_F14:
			return GLFW_KEY_F14;
		case VK_F15:
			return GLFW_KEY_F15;
		case VK_F16:
			return GLFW_KEY_F16;
		case VK_F17:
			return GLFW_KEY_F17;
		case VK_F18:
			return GLFW_KEY_F18;
		case VK_F19:
			return GLFW_KEY_F19;
		case VK_F20:
			return GLFW_KEY_F20;
		case VK_F21:
			return GLFW_KEY_F21;
		case VK_F22:
			return GLFW_KEY_F22;
		case VK_F23:
			return GLFW_KEY_F23;
		case VK_F24:
			return GLFW_KEY_F24;
		case VK_SPACE:
			return GLFW_KEY_SPACE;

			// Numeric keypad
		case VK_NUMPAD0:
			return GLFW_KEY_KP_0;
		case VK_NUMPAD1:
			return GLFW_KEY_KP_1;
		case VK_NUMPAD2:
			return GLFW_KEY_KP_2;
		case VK_NUMPAD3:
			return GLFW_KEY_KP_3;
		case VK_NUMPAD4:
			return GLFW_KEY_KP_4;
		case VK_NUMPAD5:
			return GLFW_KEY_KP_5;
		case VK_NUMPAD6:
			return GLFW_KEY_KP_6;
		case VK_NUMPAD7:
			return GLFW_KEY_KP_7;
		case VK_NUMPAD8:
			return GLFW_KEY_KP_8;
		case VK_NUMPAD9:
			return GLFW_KEY_KP_9;
		case VK_DIVIDE:
			return GLFW_KEY_KP_DIVIDE;
		case VK_MULTIPLY:
			return GLFW_KEY_KP_MULTIPLY;
		case VK_SUBTRACT:
			return GLFW_KEY_KP_SUBTRACT;
		case VK_ADD:
			return GLFW_KEY_KP_ADD;
		case VK_DECIMAL:
			return GLFW_KEY_KP_DECIMAL;
		case VK_NUMLOCK:
			return GLFW_KEY_NUM_LOCK;

		case VK_CAPITAL:
			return GLFW_KEY_CAPS_LOCK;
		case VK_SCROLL:
			return GLFW_KEY_SCROLL_LOCK;
		case VK_PAUSE:
			return GLFW_KEY_PAUSE;

		case VK_LWIN:
			return GLFW_KEY_LEFT_SUPER;
		case VK_RWIN:
			return GLFW_KEY_RIGHT_SUPER;
		case VK_APPS:
			return GLFW_KEY_MENU;

			// The rest (should be printable keys)
		default:
		{
			// Convert to printable character (ISO-8859-1 or Unicode)
			wParam = MapVirtualKey((UINT)wParam, 2) & 0x0000FFFF;

			// Make sure that the character is uppercase
			// Note: Old Windows versions are not supported
			wParam = (WPARAM)CharUpperW((LPWSTR)wParam);

			// Valid ISO-8859-1 character?
			if ((wParam >= 32 && wParam <= 126) || (wParam >= 160 && wParam <= 255))
			{
				return (int)wParam;
			}

			return GLFW_KEY_UNKNOWN;
		}
	}
}

//
// Now, own code
//

static HDC _nativeDisplay = EGL_DEFAULT_DISPLAY;

static HWND _nativeWindow = 0;

static GLUSint _width = -1;

static GLUSint _height = -1;

static GLUSint _wheelPos = 0;

static GLUSboolean _fullscreen = GLUS_FALSE;

LRESULT CALLBACK _glusProcessWindow(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
		case WM_CLOSE:
		{
			PostQuitMessage(0);

			return 0;
		}
		case WM_SIZE:
		{
			_width = LOWORD(lParam);
			_height = HIWORD(lParam);

			glusInternalReshape(_width, _height);

			return 0;
		}
		case WM_MOUSEMOVE:
		{
			GLUSint mouseX, mouseY;

			mouseX = (GLUSint)((short)LOWORD(lParam));
			mouseY = (GLUSint)((short)HIWORD(lParam));

			glusInternalMouseMove(mouseX, mouseY);

			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			GLUSint wheelDelta = (((int)wParam) >> 16) / WHEEL_DELTA;

			_wheelPos += wheelDelta;

			glusInternalMouseWheel(_wheelPos);

			return 0;
		}
		case WM_LBUTTONDOWN:
		{
			SetCapture(hWnd);
			_glusInputMouseClick(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);

			return 0;
		}
		case WM_LBUTTONUP:
		{
			ReleaseCapture();
			_glusInputMouseClick(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);

			return 0;
		}
		case WM_MBUTTONDOWN:
		{
			SetCapture(hWnd);
			_glusInputMouseClick(GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS);

			return 0;
		}
		case WM_MBUTTONUP:
		{
			ReleaseCapture();
			_glusInputMouseClick(GLFW_MOUSE_BUTTON_MIDDLE, GLFW_RELEASE);

			return 0;
		}
			return 0;
		case WM_RBUTTONDOWN:
		{
			SetCapture(hWnd);
			_glusInputMouseClick(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);

			return 0;
		}
		case WM_RBUTTONUP:
		{
			ReleaseCapture();
			_glusInputMouseClick(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE);

			return 0;
		}
		case WM_KEYDOWN:
		case WM_SYSKEYDOWN:
		{
			_glusInputKey(_glusTranslateKey(wParam, lParam), GLFW_PRESS);

			break;
		}
		case WM_KEYUP:
		case WM_SYSKEYUP:
		{
			_glusInputKey(_glusTranslateKey(wParam, lParam), GLFW_RELEASE);

			break;
		}
	}

	return DefWindowProc(hWnd, uiMsg, wParam, lParam);
}

GLUSvoid _glusPollEvents()
{
	MSG msg;

	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			glusInternalClose();
		}
		else
		{
			DispatchMessage(&msg);
		}
	}
}

EGLNativeDisplayType _glusGetNativeDisplayType()
{
	return _nativeDisplay;
}

EGLNativeWindowType _glusCreateNativeWindowType(const char* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, const GLUSint nativeVisualID)
{
	WNDCLASS wc;
	RECT wRect;
	HINSTANCE hInstance;

	DWORD dwStyle, dwExStyle;

	GLUSint fullWidth, fullHeight;

	dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_VISIBLE;
	dwExStyle = WS_EX_APPWINDOW;

	if (fullscreen)
	{
		DEVMODE dm;

		dm.dmSize = sizeof(DEVMODE);
		if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &dm))
		{
			glusLogPrint(GLUS_LOG_ERROR, "Could not enumerate display settings");

			return 0;
		}
		dm.dmPelsWidth = width;
		dm.dmPelsHeight = height;
		dm.dmFields = DM_PELSWIDTH | DM_PELSHEIGHT;

		if (ChangeDisplaySettings(&dm, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
		{
			glusLogPrint(GLUS_LOG_ERROR, "Could not switch to full screen: ", width, height);

			return 0;
		}

		dwStyle |= WS_POPUP;
	}
	else
	{
		dwStyle |= WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

		if (!noResize)
		{
			dwStyle |= WS_MAXIMIZEBOX | WS_SIZEBOX;
			dwExStyle |= WS_EX_WINDOWEDGE;
		}
	}

	hInstance = GetModuleHandle(NULL);

	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = (WNDPROC)_glusProcessWindow;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = NULL;
	wc.lpszMenuName = NULL;
	wc.lpszClassName = "GLES3";

	RegisterClass(&wc);

	//

	wRect.left = 0L;
	wRect.right = (long)width - 1;
	wRect.top = 0L;
	wRect.bottom = (long)height - 1;

	AdjustWindowRectEx(&wRect, dwStyle, FALSE, dwExStyle);

	fullWidth = wRect.right - wRect.left + 1;
	fullHeight = wRect.bottom - wRect.top + 1;

	if (fullscreen)
	{
		wRect.left = wRect.top = 0;
	}
	else
	{
		SystemParametersInfo(SPI_GETWORKAREA, 0, &wRect, 0);
	}
	//

	_nativeWindow = CreateWindowEx(dwExStyle, "GLES3", title, dwStyle, wRect.left, wRect.top, fullWidth, fullHeight, NULL, NULL, hInstance, NULL);

	ShowWindow(_nativeWindow, SW_SHOW);
	SetForegroundWindow(_nativeWindow);
	SetFocus(_nativeWindow);

	_fullscreen = fullscreen;

	_width = width;
	_height = height;

	return _nativeWindow;
}

GLUSvoid _glusDestroyNativeWindow()
{
	if (_nativeWindow)
	{
		DestroyWindow(_nativeWindow);

		_nativeWindow = 0;
	}

	if (_fullscreen)
	{
		ChangeDisplaySettings(NULL, CDS_FULLSCREEN);

		_fullscreen = GLUS_FALSE;
	}
}

double _glusGetRawTime()
{
	static LONGLONG perfromanceFrequency = 0;

	LONGLONG currentPerformanceCounter;

	if (perfromanceFrequency == 0)
	{
		QueryPerformanceFrequency((LARGE_INTEGER*)&perfromanceFrequency);
	}

	QueryPerformanceCounter((LARGE_INTEGER*)&currentPerformanceCounter);

	return (double)currentPerformanceCounter / (double)perfromanceFrequency;
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
