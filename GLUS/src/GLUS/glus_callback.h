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

#ifndef GLUS_CALLBACK_H_
#define GLUS_CALLBACK_H_

/**
 * Sets the users initialization function. Called before the reshape and update functions.
 *
 * The function must return GLUS_TRUE, if the initialization succeeded. If not, the program terminates.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusCallbackSetInitFunc(GLUSboolean(*glusNewInit)(GLUSvoid));

/**
 * Sets the users reshape function. Called after the initialization and before the update function.
 * If the window is resized, this function is also called before the update function.
 *
 * The function receives the current width and height of the resized window.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusCallbackSetReshapeFunc(GLUSvoid(*glusNewReshape)(const GLUSint width, const GLUSint height));

/**
 * Sets the users update function, which is called every frame.
 *
 * The time difference since last frame is passed in seconds and the resolution is at least in milliseconds.
 *
 * If the function does not return GLUS_TRUE, the application is terminated.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusCallbackSetUpdateFunc(GLUSboolean(*glusNewUpdate)(const GLUSfloat time));

/**
 * Sets the users terminate function, which is called in any case. It can be used to clean up resources.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusCallbackSetTerminateFunc(GLUSvoid(*glusNewTerminate)(GLUSvoid));

/**
 * Sets the users key handler.
 *
 * The function receives the values, if a key was pressed or released.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusCallbackSetKeyFunc(GLUSvoid(*glusNewKey)(const GLUSboolean pressed, const GLUSint key));

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
GLUSAPI GLUSvoid GLUSAPIENTRY glusCallbackSetMouseFunc(GLUSvoid(*glusNewMouse)(const GLUSboolean pressed, const GLUSint button, const GLUSint xPos, const GLUSint yPos));

/**
 * Sets the users mouse wheel function.
 *
 * The function receives the states of all buttons, the direction of the wheel in ticks and the current x and y position.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusCallbackSetMouseWheelFunc(GLUSvoid(*glusNewMouseWheel)(const GLUSint buttons, const GLUSint ticks, const GLUSint xPos, const GLUSint yPos));

/**
 * Sets the users mouse move function.
 *
 * The function receives the states of all buttons and the current x and y position.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusCallbackSetMouseMoveFunc(GLUSvoid(*glusNewMouseMove)(const GLUSint buttons, const GLUSint xPos, const GLUSint yPos));

#endif /* GLUS_CALLBACK_H_ */
