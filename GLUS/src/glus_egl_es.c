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

EGLBoolean GLUSAPIENTRY glusEGLGetDisplayChooseConfig(EGLNativeDisplayType eglNativeDisplayType, EGLDisplay* eglDisplay, EGLConfig* eglConfig, const EGLint attribList[])
{
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLConfig config = 0;

    if (!eglDisplay || !eglConfig)
    {
        glusLogPrint(GLUS_LOG_ERROR, "No eglDisplay or eglConfig passed");

        return EGL_FALSE;
    }

    // Get Display
    display = eglGetDisplay((EGLNativeDisplayType) eglNativeDisplayType);
    if (display == EGL_NO_DISPLAY)
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not get EGL display");

        return EGL_FALSE;
    }

    // Initialize EGL
    if (!eglInitialize(display, &majorVersion, &minorVersion))
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not initialize EGL");

        glusEGLTerminate(display, 0, 0);

        return EGL_FALSE;
    }

    glusLogPrint(GLUS_LOG_INFO, "EGL version: %d.%d", majorVersion, minorVersion);

    // Choose config
    if (!eglChooseConfig(display, attribList, &config, 1, &numConfigs))
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not choose EGL configuration");

        glusEGLTerminate(display, 0, 0);

        return EGL_FALSE;
    }

    if (numConfigs == 0)
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could no EGL configuration returned");

        glusEGLTerminate(display, 0, 0);

        return EGL_FALSE;
    }

    *eglDisplay = display;
    *eglConfig = config;

    return EGL_TRUE;
}

EGLBoolean GLUSAPIENTRY glusEGLGetNativeVisualID(EGLDisplay eglDisplay, EGLConfig eglConfig, EGLint* eglNativeVisualID)
{
	return eglGetConfigAttrib(eglDisplay, eglConfig, EGL_NATIVE_VISUAL_ID, eglNativeVisualID);
}

EGLBoolean GLUSAPIENTRY glusEGLCreateWindowSetContext(EGLNativeWindowType eglNativeWindowType, EGLint eglContextClientVersion, EGLDisplay* eglDisplay, EGLConfig* eglConfig, EGLSurface* eglSurface, EGLContext* eglContext)
{
    EGLDisplay surface = EGL_NO_SURFACE;
    EGLDisplay context = EGL_NO_CONTEXT;

    EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, GLUS_DEFAULT_CLIENT_VERSION, EGL_NONE, EGL_NONE };

    contextAttribs[1] = eglContextClientVersion;

    if (!eglDisplay || !eglConfig || !eglSurface || !eglContext)
    {
        glusLogPrint(GLUS_LOG_ERROR, "No eglDisplay, eglSurface or eglContext passed");

        return EGL_FALSE;
    }

    // Create a surface
    surface = eglCreateWindowSurface(*eglDisplay, *eglConfig, (EGLNativeWindowType) eglNativeWindowType, 0);
    if (surface == EGL_NO_SURFACE)
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not create EGL window surface");

        glusEGLTerminate(eglDisplay, eglSurface, eglContext);

        return EGL_FALSE;
    }

    // Create a GL context
    context = eglCreateContext(*eglDisplay, *eglConfig, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT)
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not create EGL context. Context client version: %d", eglContextClientVersion);

        glusEGLTerminate(eglDisplay, eglSurface, eglContext);

        return EGL_FALSE;
    }

    glusLogPrint(GLUS_LOG_INFO, "EGL context client version: %d", eglContextClientVersion);

    // Make the context current
    if (!eglMakeCurrent(*eglDisplay, surface, surface, context))
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not set EGL context as current");

        glusEGLTerminate(eglDisplay, eglSurface, eglContext);

        return EGL_FALSE;
    }

    *eglSurface = surface;
    *eglContext = context;

    return EGL_TRUE;
}

GLvoid GLUSAPIENTRY glusEGLTerminate(EGLDisplay* eglDisplay, EGLSurface* eglSurface, EGLContext* eglContext)
{
    if (!eglDisplay)
    {
    	glusLogPrint(GLUS_LOG_ERROR, "No eglDisplay passed");

        return;
    }

    if (*eglDisplay)
    {
        eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (eglContext && *eglContext)
        {
            eglDestroyContext(eglDisplay, *eglContext);

            *eglContext = EGL_NO_CONTEXT;
        }

        if (eglSurface && *eglSurface)
        {
            eglDestroySurface(eglDisplay, *eglSurface);

            *eglSurface = EGL_NO_SURFACE;
        }

        eglTerminate(*eglDisplay);

        *eglDisplay = EGL_NO_DISPLAY;
    }
}

