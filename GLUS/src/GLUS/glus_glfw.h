/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) 2010 - 2014 Norbert Nopper
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

#ifndef GLUS_GLFW_H_
#define GLUS_GLFW_H_

/**
 * Creates the window. In this function, mainly GLEW and GLFW functions are used. By default, a RGBA color buffer is created.
 *
 * @param title				Title of the window.
 * @param width				Width of the window.
 * @param height			Height of the window.
 * @param fullscreen 		Flag for setting the window to fullscreen.
 * @param noResize 			GLUS_TRUE, if it should not be possible to resize the window.
 * @param configAttribList 	EGL configuration attribute list.
 * @param contextAttribList EGL context attribute list.
 *
 * @return GLUS_TRUE, if creation of OpenGL context and window succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusCreateWindow(const GLUSchar* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, const EGLint* configAttribList, const EGLint* contextAttribList);

/**
 * Cleans up the window and frees all resources. Only needs to be called, if creation of the window failed.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusDestroyWindow(GLUSvoid);

/**
 * Starts the main loop. First the init, reshape, update and - if terminated - terminate functions are called.
 *
 * @return GLUS_ TRUE, if running the main loop succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusRun(GLUSvoid);

/**
 * Starts everything before the main loop. The init and reshape functions are called.
 * Use this function, if glusRun can not be used.
 *
 * @return GLUS_ TRUE, if startup succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusStartup(GLUSvoid);

/**
 * Calls the main loop once. Update function is called.
 * Use this function, if glusRun can not be used.
 *
 * @return GLUS_ TRUE, if looping should continue.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusLoop(GLUSvoid);

/**
 * Called after the main loop. The terminate function is called.
 * Use this function, if glusRun can not be used.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusShutdown(GLUSvoid);

/**
 * Specifies the minimum number of video frame periods per buffer swap for the window.
 * Has to be called after the window is created.
 *
 * @param interval Specifies the minimum number of video frames that are displayed before a buffer swap will occur.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusSwapInterval(GLUSint interval);

#endif /* GLUS_GLFW_H_ */
