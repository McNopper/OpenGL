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

#include "GL/glus.h"

EGLBoolean GLUSAPIENTRY glusEGLCreateContext(EGLNativeDisplayType eglNativeDisplayType, EGLDisplay* eglDisplay, EGLConfig* eglConfig, EGLContext *eglContext, const EGLint configAttribList[], const EGLint contextAttribList[])
{
    EGLint numConfigs;
    EGLint majorVersion;
    EGLint minorVersion;
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLConfig config = 0;
    EGLDisplay context = EGL_NO_CONTEXT;

    if (!eglDisplay || !eglConfig || !eglContext)
    {
        glusLogPrint(GLUS_LOG_ERROR, "No eglDisplay, eglConfig or eglContext passed");

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

    if (!eglBindAPI(GLUS_EGL_API))
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not bind API");

        glusEGLTerminate(display, 0, 0);

        return EGL_FALSE;
    }

    // Choose config
    if (!eglChooseConfig(display, configAttribList, &config, 1, &numConfigs))
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not choose EGL configuration");

        glusEGLTerminate(display, 0, 0);

        return EGL_FALSE;
    }

    if (numConfigs == 0)
    {
        glusLogPrint(GLUS_LOG_ERROR, "No EGL configuration returned");

        glusEGLTerminate(display, 0, 0);

        return EGL_FALSE;
    }

    // Create a GL ES or VG context
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttribList);
    if (context == EGL_NO_CONTEXT)
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not create EGL context");

        glusEGLTerminate(eglDisplay, 0, 0);

        return EGL_FALSE;
    }

    *eglDisplay = display;
    *eglConfig = config;
    *eglContext = context;

    return EGL_TRUE;
}

EGLBoolean GLUSAPIENTRY glusEGLCreateWindowSurfaceMakeCurrent(EGLNativeWindowType eglNativeWindowType, EGLDisplay* eglDisplay, EGLConfig* eglConfig, EGLContext* eglContext, EGLSurface* eglSurface)
{
    EGLDisplay surface = EGL_NO_SURFACE;

    if (!eglDisplay || !eglConfig || !eglContext || !eglSurface)
    {
        glusLogPrint(GLUS_LOG_ERROR, "No eglDisplay, eglConfig, eglContext or eglSurface passed");

        return EGL_FALSE;
    }

    // Create a surface
    surface = eglCreateWindowSurface(*eglDisplay, *eglConfig, (EGLNativeWindowType) eglNativeWindowType, 0);
    if (surface == EGL_NO_SURFACE)
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not create EGL window surface");

        glusEGLTerminate(eglDisplay, eglContext, 0);

        return EGL_FALSE;
    }

    // Make the context current
    if (!eglMakeCurrent(*eglDisplay, surface, surface, *eglContext))
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not set EGL context as current");

        glusEGLTerminate(eglDisplay, eglContext, 0);

        return EGL_FALSE;
    }

    *eglSurface = surface;

    return EGL_TRUE;
}

GLUSvoid GLUSAPIENTRY glusEGLTerminate(EGLDisplay* eglDisplay, EGLContext* eglContext, EGLSurface* eglSurface)
{
    if (!eglDisplay)
    {
    	glusLogPrint(GLUS_LOG_ERROR, "No eglDisplay passed");

        return;
    }

    if (*eglDisplay)
    {
        eglMakeCurrent(*eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);

        if (eglSurface && *eglSurface)
        {
            eglDestroySurface(*eglDisplay, *eglSurface);

            *eglSurface = EGL_NO_SURFACE;
        }

        if (eglContext && *eglContext)
        {
            eglDestroyContext(*eglDisplay, *eglContext);

            *eglContext = EGL_NO_CONTEXT;
        }

        eglTerminate(*eglDisplay);

        *eglDisplay = EGL_NO_DISPLAY;
    }
}

