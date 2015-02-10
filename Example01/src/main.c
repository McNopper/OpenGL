/**
 * OpenGL 3 - Example 01
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

/**
 * Function for initialization.
 */
GLUSboolean init(GLUSvoid)
{
    // The background will just cleared with blue color.
    glClearColor(0.0f, 0.0f, 1.0f, 0.0f);

    return GLUS_TRUE;
}

/**
 * Function is called before first update and every time when the window is resized.
 *
 * @param w	width of the window
 * @param h	height of the window
 */
GLUSvoid reshape(GLUSint width, GLUSint height)
{
    // Set the viewport depending on the width and height of the window.
    glViewport(0, 0, width, height);
}

/**
 * Function to render and display content. Swapping of the buffers is automatically done.
 *
 * @return true for continuing, false to exit the application
 */
GLUSboolean update(GLUSfloat time)
{
    // Now, the background is painted blue.
    glClear(GL_COLOR_BUFFER_BIT);

    return GLUS_TRUE;
}

/**
 * Function to clean up things.
 */
GLUSvoid terminate(GLUSvoid)
{
}

/**
 * Main entry point.
 */
int main(int argc, char* argv[])
{
	EGLint eglConfigAttributes[] = {
	        EGL_RED_SIZE, 8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE, 8,
	        EGL_DEPTH_SIZE, 0,
	        EGL_STENCIL_SIZE, 0,
	        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	        EGL_NONE
	};

    EGLint eglContextAttributes[] = {
    		EGL_CONTEXT_MAJOR_VERSION, 3,
    		EGL_CONTEXT_MINOR_VERSION, 2,
    		EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
    		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    		EGL_NONE
    };

    glusWindowSetInitFunc(init);

    glusWindowSetReshapeFunc(reshape);

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

    if (!glusWindowCreate("GLUS Example Window", 640, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
