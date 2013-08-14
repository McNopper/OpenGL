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

#include <pthread.h>

#include <stdint.h>

#include <time.h>

#include <bcm_host.h>

#include <SDL/SDL.h>

extern GLUSint glusInternalClose(GLUSvoid);

extern GLUSvoid glusInternalKey(GLUSint key, GLUSint state);

extern GLUSvoid glusInternalMouse(GLUSint button, GLUSint action);

extern GLUSvoid glusInternalMouseWheel(GLUSint pos);

extern GLUSvoid glusInternalMouseMove(GLUSint x, GLUSint y);

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

	while (!doBreak)
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

// Map, if possible, to GLFW keys

static int translateKey(SDLKey key)
{

	switch (key)
	{
		case SDLK_SPACE:
			return GLFW_KEY_SPACE;
		case SDLK_ESCAPE:
			return GLFW_KEY_ESC;
		case SDLK_F1:
			return GLFW_KEY_F1;
		case SDLK_F2:
			return GLFW_KEY_F2;
		case SDLK_F3:
			return GLFW_KEY_F3;
		case SDLK_F4:
			return GLFW_KEY_F4;
		case SDLK_F5:
			return GLFW_KEY_F5;
		case SDLK_F6:
			return GLFW_KEY_F6;
		case SDLK_F7:
			return GLFW_KEY_F7;
		case SDLK_F8:
			return GLFW_KEY_F8;
		case SDLK_F9:
			return GLFW_KEY_F9;
		case SDLK_F10:
			return GLFW_KEY_F10;
		case SDLK_F11:
			return GLFW_KEY_F11;
		case SDLK_F12:
			return GLFW_KEY_F12;
		case SDLK_F13:
			return GLFW_KEY_F13;
		case SDLK_F14:
			return GLFW_KEY_F14;
		case SDLK_F15:
			return GLFW_KEY_F15;
		case SDLK_UP:
			return GLFW_KEY_UP;
		case SDLK_DOWN:
			return GLFW_KEY_DOWN;
		case SDLK_LEFT:
			return GLFW_KEY_LEFT;
		case SDLK_RIGHT:
			return GLFW_KEY_RIGHT;
		case SDLK_LSHIFT:
			return GLFW_KEY_LSHIFT;
		case SDLK_RSHIFT:
			return GLFW_KEY_RSHIFT;
		case SDLK_LCTRL:
			return GLFW_KEY_LCTRL;
		case SDLK_RCTRL:
			return GLFW_KEY_RCTRL;
		case SDLK_LALT:
			return GLFW_KEY_LALT;
		case SDLK_RALT:
			return GLFW_KEY_RALT;
		case SDLK_TAB:
			return GLFW_KEY_TAB;
		case SDLK_RETURN:
			return GLFW_KEY_ENTER;
		case SDLK_BACKSPACE:
			return GLFW_KEY_BACKSPACE;
		case SDLK_INSERT:
			return GLFW_KEY_INSERT;
		case SDLK_DELETE:
			return GLFW_KEY_DEL;
		case SDLK_PAGEUP:
			return GLFW_KEY_PAGEUP;
		case SDLK_PAGEDOWN:
			return GLFW_KEY_PAGEDOWN;
		case SDLK_HOME:
			return GLFW_KEY_HOME;
		case SDLK_END:
			return GLFW_KEY_END;
		case SDLK_KP0:
			return GLFW_KEY_KP_0;
		case SDLK_KP1:
			return GLFW_KEY_KP_1;
		case SDLK_KP2:
			return GLFW_KEY_KP_2;
		case SDLK_KP3:
			return GLFW_KEY_KP_3;
		case SDLK_KP4:
			return GLFW_KEY_KP_4;
		case SDLK_KP5:
			return GLFW_KEY_KP_5;
		case SDLK_KP6:
			return GLFW_KEY_KP_6;
		case SDLK_KP7:
			return GLFW_KEY_KP_7;
		case SDLK_KP8:
			return GLFW_KEY_KP_8;
		case SDLK_KP9:
			return GLFW_KEY_KP_9;
		case SDLK_KP_DIVIDE:
			return GLFW_KEY_KP_DIVIDE;
		case SDLK_KP_MULTIPLY:
			return GLFW_KEY_KP_MULTIPLY;
		case SDLK_KP_MINUS:
			return GLFW_KEY_KP_SUBTRACT;
		case SDLK_KP_PLUS:
			return GLFW_KEY_KP_ADD;
		case SDLK_KP_PERIOD:
			return GLFW_KEY_KP_DECIMAL;
		case SDLK_KP_EQUALS:
			return GLFW_KEY_KP_EQUAL;
		case SDLK_KP_ENTER:
			return GLFW_KEY_KP_ENTER;
		case SDLK_NUMLOCK:
			return GLFW_KEY_KP_NUM_LOCK;
		case SDLK_CAPSLOCK:
			return GLFW_KEY_CAPS_LOCK;
		case SDLK_SCROLLOCK:
			return GLFW_KEY_SCROLL_LOCK;
		case SDLK_PAUSE:
			return GLFW_KEY_PAUSE;
		case SDLK_LSUPER:
			return GLFW_KEY_LSUPER;
		case SDLK_RSUPER:
			return GLFW_KEY_RSUPER;
		case SDLK_MENU:
			return GLFW_KEY_MENU;
		default:
			return (int)key;
	}

	return GLFW_KEY_UNKNOWN;
}

//

static GLUSboolean _nativeWindowCreated = GLUS_FALSE;

static DISPMANX_DISPLAY_HANDLE_T _nativeDisplay = 0;

static EGL_DISPMANX_WINDOW_T _nativeWindow;

static GLUSint _width = -1;

static GLUSint _height = -1;

static GLUSint _wheelPos = 0;

static GLUSboolean _fullscreen = GLUS_FALSE;

GLUSvoid _glusPollEvents()
{
	SDL_Event event;

	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_KEYDOWN:
			{
				// CTRL-C
				if (event.key.keysym.sym == 99 && (event.key.keysym.mod == KMOD_LCTRL || event.key.keysym.mod == KMOD_RCTRL))
				{
					glusInternalClose();

					return;
				}

				glusInternalKey(translateKey(event.key.keysym.sym), GLUS_TRUE);
			}
			break;

			case SDL_KEYUP:
			{
				glusInternalKey(translateKey(event.key.keysym.sym), GLUS_FALSE);
			}
			break;

			case SDL_MOUSEMOTION:
			{
				if (event.motion.x <= _width && event.motion.y <= _height)
				{
					glusInternalMouseMove(event.motion.x, event.motion.y);
				}
			}
			break;

			case SDL_MOUSEBUTTONDOWN:
			{
				if (event.button.x <= _width && event.button.y <= _height)
				{
					switch (event.button.button)
					{
						case 1:
							glusInternalMouse(GLFW_MOUSE_BUTTON_LEFT, GLFW_PRESS);
						break;
						case 2:
							glusInternalMouse(GLFW_MOUSE_BUTTON_MIDDLE, GLFW_PRESS);
						break;
						case 3:
							glusInternalMouse(GLFW_MOUSE_BUTTON_RIGHT, GLFW_PRESS);
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
				}
			}
			break;

			case SDL_MOUSEBUTTONUP:
			{
				if (event.button.x <= _width && event.button.y <= _height)
				{
					switch (event.button.button)
					{
						case 1:
							glusInternalMouse(GLFW_MOUSE_BUTTON_LEFT, GLFW_RELEASE);
						break;
						case 2:
							glusInternalMouse(GLFW_MOUSE_BUTTON_MIDDLE, GLFW_RELEASE);
						break;
						case 3:
							glusInternalMouse(GLFW_MOUSE_BUTTON_RIGHT, GLFW_RELEASE);
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
				}
			}
			break;
		}
	}
}

EGLNativeDisplayType _glusGetNativeDisplayType()
{
	return EGL_DEFAULT_DISPLAY ;
}

EGLNativeWindowType _glusCreateNativeWindowType(const char* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize)
{
	const SDL_VideoInfo* videoInfo;

	//

	DISPMANX_UPDATE_HANDLE_T dispmanUpdate;
	DISPMANX_ELEMENT_HANDLE_T dispmanElement;
	VC_RECT_T dstRect;
	VC_RECT_T srcRect;
	VC_DISPMANX_ALPHA_T dispmanAlpha;
	int32_t success;
	int32_t windowWidth;
	int32_t windowHeight;

	glusLogPrint(GLUS_LOG_INFO, "Parameters 'title' and 'noResize' are not used");

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

			return EGL_NO_SURFACE ;
		}

		vc_tv_register_callback(resizeDone, 0);

		if (vc_tv_hdmi_power_on_explicit_new(group, supportedModes[i].group, supportedModes[i].code) != 0)
		{
			vc_tv_unregister_callback(resizeDone);

			glusLogPrint(GLUS_LOG_ERROR, "Could not switch to full screen: ", width, height);

			return EGL_NO_SURFACE ;
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

		return EGL_NO_SURFACE ;
	}

	//

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Could not initialize SDL");

		return EGL_NO_SURFACE ;
	}

	videoInfo = SDL_GetVideoInfo();

	if (!videoInfo)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Could not get video info for SDL");

		return EGL_NO_SURFACE ;
	}

	if (!SDL_SetVideoMode(videoInfo->current_w, videoInfo->current_h, videoInfo->vfmt->BitsPerPixel, SDL_HWSURFACE))
	{
		glusLogPrint(GLUS_LOG_ERROR, "Set video mode for SDL failed");

		return EGL_NO_SURFACE ;
	}

	SDL_ShowCursor(SDL_DISABLE);

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

	dispmanElement = vc_dispmanx_element_add(dispmanUpdate, _nativeDisplay, 0 /*layer*/, &dstRect, 0 /*src*/, &srcRect, DISPMANX_PROTECTION_NONE, &dispmanAlpha, 0/*clamp*/, 0/*transform*/);

	success = vc_dispmanx_update_submit_sync(dispmanUpdate);

	if (!dispmanElement || success < 0)
	{
		glusLogPrint(GLUS_LOG_ERROR, "Could not add element");

		return EGL_NO_SURFACE ;
	}

	_width = windowWidth;
	_height = windowHeight;

	_nativeWindow.element = dispmanElement;
	_nativeWindow.width = windowWidth;
	_nativeWindow.height = windowHeight;

	_nativeWindowCreated = GLUS_TRUE;

	return (EGLNativeWindowType)&_nativeWindow;
}

GLUSvoid _glusDestroyNativeWindow()
{
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

	SDL_ShowCursor(SDL_ENABLE);

	SDL_Quit();

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
