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

#ifndef GLUS_EGL_H_
#define GLUS_EGL_H_

/**
 * Creates the context and all needed EGL elements.
 *
 * @param eglNativeDisplayType		Native display type.
 * @param eglDisplay  				EGL display.
 * @param eglConfig  				EGL configuration.
 * @param eglContext 				EGL context.
 * @param configAttribList 			EGL configuration attribute list.
 * @param contextAttribList 		EGL context attribute list.
 *
 * @return EGL_TRUE, when creation of context succeeded.
 */
GLUSAPI EGLBoolean GLUSAPIENTRY glusEGLCreateContext(EGLNativeDisplayType eglNativeDisplayType, EGLDisplay* eglDisplay, EGLConfig* eglConfig, EGLContext *eglContext, const EGLint configAttribList[], const EGLint contextAttribList[]);

/**
 * Creates a window surface and sets it as current.
 *
 * @param eglNativeWindowType   	Native window type.
 * @param eglDisplay 				EGL display.
 * @param eglConfig  				EGL configuration.
 * @param eglContext 				EGL context.
 * @param eglSurface 				EGL surface.
 *
 * @return EGL_TRUE, when creation of window and setting of context succeeded.
 */
GLUSAPI EGLBoolean GLUSAPIENTRY glusEGLCreateWindowSurfaceMakeCurrent(EGLNativeWindowType eglNativeWindowType, EGLDisplay* eglDisplay, EGLConfig* eglConfig, EGLContext* eglContext, EGLSurface* eglSurface, const EGLint* surfaceAttribList);

/**
 * Returns the created default EGL display.
 *
 * @return the EGL display.
 */
GLUSAPI EGLDisplay GLUSAPIENTRY glusEGLGetDefaultDisplay(GLUSvoid);

/**
 * Returns the created default EGL surface.
 *
 * @return the EGL surface.
 */
GLUSAPI EGLSurface GLUSAPIENTRY glusEGLGetDefaultSurface(GLUSvoid);

/**
 * Returns the created default EGL context.
 *
 * @return the EGL context.
 */
GLUSAPI EGLContext GLUSAPIENTRY glusEGLGetDefaultContext(GLUSvoid);

/**
 * Terminates EGL an frees all resources.
 *
 * @param eglDisplay EGL display.
 * @param eglContext EGL context.
 * @param eglSurface EGL surface.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusEGLTerminate(EGLDisplay* eglDisplay, EGLContext* eglContext, EGLSurface* eglSurface);

#endif /* GLUS_EGL_H_ */
