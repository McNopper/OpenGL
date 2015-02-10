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

#ifndef GLUS_WINDOW_H_
#define GLUS_WINDOW_H_

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
 * @param surfaceAttribList EGL surface attribute list.
 *
 * @return GLUS_TRUE, if creation of OpenGL context and window succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusWindowCreate(const GLUSchar* title, const GLUSint width, const GLUSint height, const GLUSboolean fullscreen, const GLUSboolean noResize, const EGLint* configAttribList, const EGLint* contextAttribList, const EGLint* surfaceAttribList);

/**
 * Cleans up the window and frees all resources. Only needs to be called, if creation of the window failed.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowDestroy(GLUSvoid);

/**
 * Starts the main loop. First the init, reshape, update and - if terminated - terminate functions are called.
 *
 * @return GLUS_ TRUE, if running the main loop succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusWindowRun(GLUSvoid);

/**
 * Starts everything before the main loop. The init and reshape functions are called.
 * Use this function, if glusRun can not be used.
 *
 * @return GLUS_ TRUE, if startup succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusWindowStartup(GLUSvoid);

/**
 * Calls the main loop once. Update function is called.
 * Use this function, if glusRun can not be used.
 *
 * @return GLUS_ TRUE, if looping should continue.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusWindowLoop(GLUSvoid);

/**
 * Calls the main loop once. Update function is called and one screenshot is taken and stored.
 * Use this function, if glusRun can not be used.
 *
 * @return GLUS_ TRUE, if looping should continue.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusWindowLoopDoRecording(GLUSvoid);

/**
 * Called after the main loop. The terminate function is called.
 * Use this function, if glusRun can not be used.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowShutdown(GLUSvoid);

/**
 * Specifies the minimum number of video frame periods per buffer swap for the window.
 * Has to be called after the window is created.
 *
 * @param interval Specifies the minimum number of video frames that are displayed before a buffer swap will occur.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowSwapInterval(GLUSint interval);

//

/**
 * Sets the users initialization function. Called before the reshape and update functions.
 *
 * The function must return GLUS_TRUE, if the initialization succeeded. If not, the program terminates.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowSetInitFunc(GLUSboolean(*glusNewInit)(GLUSvoid));

/**
 * Sets the users reshape function. Called after the initialization and before the update function.
 * If the window is resized, this function is also called before the update function.
 *
 * The function receives the current width and height of the resized window.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowSetReshapeFunc(GLUSvoid(*glusNewReshape)(const GLUSint width, const GLUSint height));

/**
 * Sets the users update function, which is called every frame.
 *
 * The time difference since last frame is passed in seconds and the resolution is at least in milliseconds.
 *
 * If the function does not return GLUS_TRUE, the application is terminated.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowSetUpdateFunc(GLUSboolean(*glusNewUpdate)(const GLUSfloat time));

/**
 * Sets the users terminate function, which is called in any case. It can be used to clean up resources.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowSetTerminateFunc(GLUSvoid(*glusNewTerminate)(GLUSvoid));

/**
 * Sets the users key handler.
 *
 * The function receives the values, if a key was pressed or released.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowSetKeyFunc(GLUSvoid(*glusNewKey)(const GLUSboolean pressed, const GLUSint key));

/**
 * Sets the users mouse handler.
 *
 * The function receives the state of a button and the x and y position in the window.
 *
 * Buttons are:
 * 1 Left mouse button.
 * 2 Middle mouse button.
 * 4 Right mouse button.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowSetMouseFunc(GLUSvoid(*glusNewMouse)(const GLUSboolean pressed, const GLUSint button, const GLUSint xPos, const GLUSint yPos));

/**
 * Sets the users mouse wheel function.
 *
 * The function receives the states of all buttons, the direction of the wheel in ticks and the current x and y position.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowSetMouseWheelFunc(GLUSvoid(*glusNewMouseWheel)(const GLUSint buttons, const GLUSint ticks, const GLUSint xPos, const GLUSint yPos));

/**
 * Sets the users mouse move function.
 *
 * The function receives the states of all buttons and the current x and y position.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowSetMouseMoveFunc(GLUSvoid(*glusNewMouseMove)(const GLUSint buttons, const GLUSint xPos, const GLUSint yPos));

/**
 * Starts recording image clips, by making screenshots of the window.
 *
 * @param numberFrames		The number of frames to record.
 * @param framesPerSecond	Frames per second to use.
 *
 * @return GLUS_ TRUE, is start succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusWindowStartRecording(GLUSint numberFrames, GLUSint framesPerSecond);

/**
 * Checks, if a recording is running.
 *
 * @return GLUS_ TRUE, is recording is running.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusWindowIsRecording(GLUSvoid);

/**
 * Stops recording the image clips.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusWindowStopRecording(GLUSvoid);

/**
 * Get window width.
 *
 * @return The width of the window.
 */
GLUSAPI GLUSint GLUSAPIENTRY glusWindowGetWidth(GLUSvoid);

/**
 * Get window height.
 *
 * @return The height of the window.
 */
GLUSAPI GLUSint GLUSAPIENTRY glusWindowGetHeight(GLUSvoid);

#endif /* GLUS_WINDOW_H_ */
