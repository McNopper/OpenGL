/**
 * OpenGL 3 - Example 29
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

#define WIDTH 640
#define HEIGHT 480
#define BYTES_PER_PIXEL 3

/**
 * The used shader program.
 */
static GLUSshaderprogram g_program;

/**
 * The location of the texture in the shader program.
 */
static GLint g_textureLocation;

/**
 * The texture, which is filled with the ray traced picture.
 */
static GLuint g_texture = 0;

static GLUSvoid renderToPixelBuffer(GLubyte* pixels, GLint width, GLint height)
{
	// TODO Replace with ray tracer
	for (int i = 0; i < WIDTH * HEIGHT; i++)
	{
		pixels[i * BYTES_PER_PIXEL + 0] = 255;
		pixels[i * BYTES_PER_PIXEL + 1] = 255;
		pixels[i * BYTES_PER_PIXEL + 2] = 255;

		if (i < WIDTH * 100)
		{
			pixels[i * BYTES_PER_PIXEL + 1] = 0;
			pixels[i * BYTES_PER_PIXEL + 2] = 0;
		}
	}
}

/**
 * Function for initialization.
 */
GLUSboolean init(GLUSvoid)
{
	GLubyte pixels[WIDTH * HEIGHT * BYTES_PER_PIXEL];

	//

	GLUStextfile vertexSource;
	GLUStextfile fragmentSource;

	glusLoadTextFile("../Example29/shader/fullscreen.vert.glsl", &vertexSource);
	glusLoadTextFile("../Example29/shader/texture.frag.glsl", &fragmentSource);

	glusBuildProgramFromSource(&g_program, (const GLchar**)&vertexSource.text, 0, 0, 0, (const GLchar**)&fragmentSource.text);

	glusDestroyTextFile(&vertexSource);
	glusDestroyTextFile(&fragmentSource);

	g_textureLocation = glGetUniformLocation(g_program.program, "u_texture");

	// Render into pixel buffer

	renderToPixelBuffer(pixels, WIDTH, HEIGHT);

	// Generate texture

	glGenTextures(1, &g_texture);
	glBindTexture(GL_TEXTURE_2D, g_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glUseProgram(g_program.program);

	//

	glBindTexture(GL_TEXTURE_2D, g_texture);
	glUniform1i(g_textureLocation, 0);

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
	glViewport(0, 0, width, height);
}

/**
 * Function to render and display content. Swapping of the buffers is automatically done.
 *
 * @return true for continuing, false to exit the application
 */
GLUSboolean update(GLUSfloat time)
{
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	return GLUS_TRUE;
}

/**
 * Function to clean up things.
 */
GLUSvoid terminate(GLUSvoid)
{
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_texture)
	{
		glDeleteTextures(1, &g_texture);

		g_texture = 0;
	}

	//

	glUseProgram(0);

	glusDestroyProgram(&g_program);
}

/**
 * Main entry point.
 */
int main(int argc, char* argv[])
{
	glusInitFunc(init);

	glusReshapeFunc(reshape);

	glusUpdateFunc(update);

	glusTerminateFunc(terminate);

	glusPrepareContext(3, 2, GLUS_FORWARD_COMPATIBLE_BIT);

	// To keep the program simple, no resize of the window.
	glusPrepareNoResize(GLUS_TRUE);

	if (!glusCreateWindow("GLUS Example Window", WIDTH, HEIGHT, 0, 0, GLUS_FALSE))
	{
		printf("Could not create window!\n");
		return -1;
	}

	glusRun();

	return 0;
}
