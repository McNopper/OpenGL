/**
 * OpenGL 4 - Example 32
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <math.h>
#include <stdio.h>

#include "GL/glus.h"

#define SCREEN_WIDTH	1024
#define SCREEN_HEIGHT	768
#define MSAA_SAMPLES	4

static GLUSshaderprogram g_fullscreenProgram;

static GLint g_fullscreenTextureLocation;

static GLint g_msaaSamplesLocation;
static GLint g_exposureLocation;
static GLint g_gammaLocation;

static GLfloat g_exposure = 3.0f;
static GLfloat g_gamma = 2.2f;

//

static GLuint g_fullscreenTexture;

static GLuint g_fullscreenDepthRenderbuffer;

static GLuint g_fullscreenFBO;

//
//
//

static GLfloat g_viewProjectionMatrix[16];

//

static GLuint g_panoramaTexture;

//

static GLUSshaderprogram g_programBackground;

static GLint g_viewProjectionMatrixBackgroundLocation;

static GLint g_vertexBackgroundLocation;

static GLint g_panoramaTextureBackgroundLocation;

//

static GLuint g_verticesBackgroundVBO;

static GLuint g_indicesBackgroundVBO;

static GLuint g_vaoBackground;

//

static GLuint g_numberIndicesBackground;

GLUSboolean init(GLUSvoid)
{
	GLUSshape backgroundSphere;

	GLUShdrimage image;

	GLUStextfile vertexSource;
	GLUStextfile fragmentSource;

	glusLoadTextFile("../Example32/shader/fullscreen.vert.glsl", &vertexSource);
	glusLoadTextFile("../Example32/shader/fullscreen.frag.glsl", &fragmentSource);

	glusBuildProgramFromSource(&g_fullscreenProgram, (const GLchar**)&vertexSource.text, 0, 0, 0, (const GLchar**)&fragmentSource.text);

	glusDestroyTextFile(&vertexSource);
	glusDestroyTextFile(&fragmentSource);

	//

	g_fullscreenTextureLocation = glGetUniformLocation(g_fullscreenProgram.program, "u_fullscreenTexture");

	g_msaaSamplesLocation = glGetUniformLocation(g_fullscreenProgram.program, "u_msaaSamples");
	g_exposureLocation = glGetUniformLocation(g_fullscreenProgram.program, "u_exposure");
	g_gammaLocation = glGetUniformLocation(g_fullscreenProgram.program, "u_gamma");

	//
	//

	glusLoadTextFile("../Example32/shader/background.vert.glsl", &vertexSource);
	glusLoadTextFile("../Example32/shader/background.frag.glsl", &fragmentSource);

	glusBuildProgramFromSource(&g_programBackground, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);

	glusDestroyTextFile(&vertexSource);
	glusDestroyTextFile(&fragmentSource);

	//

	g_viewProjectionMatrixBackgroundLocation = glGetUniformLocation(g_programBackground.program, "u_viewProjectionMatrix");
	g_panoramaTextureBackgroundLocation = glGetUniformLocation(g_programBackground.program, "u_panoramaTexture");

	g_vertexBackgroundLocation = glGetAttribLocation(g_programBackground.program, "a_vertex");

	//
	// Setting up the full screen frame buffer.
	//

	glGenTextures(1, &g_fullscreenTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, g_fullscreenTexture);

	// Create MSAA texture.
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLES, GL_RGB32F, SCREEN_WIDTH, SCREEN_HEIGHT, GL_TRUE);

	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_MULTISAMPLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	// No need to access the depth buffer, so a render buffer is sufficient.

	glGenRenderbuffers(1, &g_fullscreenDepthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, g_fullscreenDepthRenderbuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLES, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//

	glGenFramebuffers(1, &g_fullscreenFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, g_fullscreenFBO);

	// Attach the color buffer ...
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, g_fullscreenTexture, 0);

	// ... and the depth buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_fullscreenDepthRenderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

		return GLUS_FALSE;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//
	//
	//

	glGenTextures(1, &g_panoramaTexture);
	glBindTexture(GL_TEXTURE_2D, g_panoramaTexture);

	printf("Loading HDR texture ... ");
	if (!glusLoadHdrImage("doge2.hdr", &image))
	{
		printf("failed!\n");

		return GLUS_FALSE;
	}
	printf("done.\n");

	glGetError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, image.width, image.height, 0, GL_RGB, GL_FLOAT, image.data);
	glusDestroyHdrImage(&image);
	if (glGetError() != GL_NO_ERROR)
	{
		printf("Error creating float texture!\n");

		return GLUS_FALSE;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glusCreateSpheref(&backgroundSphere, 500.0f, 32);
	g_numberIndicesBackground = backgroundSphere.numberIndices;

	glGenBuffers(1, &g_verticesBackgroundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
	glBufferData(GL_ARRAY_BUFFER, backgroundSphere.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)backgroundSphere.vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &g_indicesBackgroundVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, backgroundSphere.numberIndices * sizeof(GLuint), (GLuint*)backgroundSphere.indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glusDestroyShapef(&backgroundSphere);

	//

	glUseProgram(g_fullscreenProgram.program);

	glUniform1i(g_fullscreenTextureLocation, 0);
	glUniform1i(g_msaaSamplesLocation, MSAA_SAMPLES);

	//

	glUseProgram(g_programBackground.program);

	glUniform1i(g_panoramaTextureBackgroundLocation, 0);

	glGenVertexArrays(1, &g_vaoBackground);
	glBindVertexArray(g_vaoBackground);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
	glVertexAttribPointer(g_vertexBackgroundLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_vertexBackgroundLocation);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);

	//

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f);

	glEnable(GL_CULL_FACE);

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	GLfloat projectionMatrix[16];
	GLfloat viewMatrix[16];

	glViewport(0, 0, width, height);

	glusPerspectivef(projectionMatrix, 60.0f, (GLfloat)width / (GLfloat)height, 1.0f, 1000.0f);

	glusLookAtf(viewMatrix, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f, -1.0f, 0.0f, 1.0f, 0.0f);

	glusMatrix4x4Multiplyf(g_viewProjectionMatrix, projectionMatrix, viewMatrix);
}

GLUSboolean update(GLUSfloat time)
{
	// Render to FBO.

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_panoramaTexture);

	glBindFramebuffer(GL_FRAMEBUFFER, g_fullscreenFBO);

	glEnable(GL_MULTISAMPLE);

	// No clear needed, as background "clears" the screen.

	// Render the background.

	glUseProgram(g_programBackground.program);

	glBindVertexArray(g_vaoBackground);

	glUniformMatrix4fv(g_viewProjectionMatrixBackgroundLocation, 1, GL_FALSE, g_viewProjectionMatrix);

	glBindVertexArray(g_vaoBackground);

	glFrontFace(GL_CW);

	glDrawElements(GL_TRIANGLES, g_numberIndicesBackground, GL_UNSIGNED_INT, 0);

	glFrontFace(GL_CCW);

	glEnable(GL_DEPTH_TEST);

	// TODO Render model using BRDF and IBL.

	//
	//
	//

	// Render full screen to resolve the buffer: MSAA, tone mapping and gamma correction.

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	// No clear needed, as we just draw over the last content.

	glUseProgram(g_fullscreenProgram.program);

	// TODO Make controllable.
	glUniform1f(g_exposureLocation, g_exposure);
	glUniform1f(g_gammaLocation, g_gamma);

	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, g_fullscreenTexture);

	glDisable(GL_DEPTH_TEST);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_panoramaTexture)
	{
		glDeleteTextures(1, &g_panoramaTexture);

		g_panoramaTexture = 0;
	}

	//

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (g_verticesBackgroundVBO)
	{
		glDeleteBuffers(1, &g_verticesBackgroundVBO);

		g_verticesBackgroundVBO = 0;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (g_indicesBackgroundVBO)
	{
		glDeleteBuffers(1, &g_indicesBackgroundVBO);

		g_indicesBackgroundVBO = 0;
	}

	glBindVertexArray(0);

	if (g_vaoBackground)
	{
		glDeleteVertexArrays(1, &g_vaoBackground);

		g_vaoBackground = 0;
	}

	glUseProgram(0);

	glusDestroyProgram(&g_programBackground);

	glusDestroyProgram(&g_fullscreenProgram);

	//

	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_fullscreenTexture)
	{
		glDeleteTextures(1, &g_fullscreenTexture);

		g_fullscreenTexture = 0;
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (g_fullscreenDepthRenderbuffer)
	{
		glDeleteRenderbuffers(1, &g_fullscreenDepthRenderbuffer);

		g_fullscreenDepthRenderbuffer = 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (g_fullscreenFBO)
	{
		glDeleteFramebuffers(1, &g_fullscreenFBO);

		g_fullscreenFBO = 0;
	}
}

int main(int argc, char* argv[])
{
	glusInitFunc(init);

	glusReshapeFunc(reshape);

	glusUpdateFunc(update);

	glusTerminateFunc(terminate);

	glusPrepareContext(4, 1, GLUS_FORWARD_COMPATIBLE_BIT);

	// No resize, as it makes code easier.
	glusPrepareNoResize(GLUS_TRUE);

	// No MSAA here, as we render to an off screen MSAA buffer.

	if (!glusCreateWindow("GLUS Example Window", SCREEN_WIDTH, SCREEN_HEIGHT, 24, 0, GLUS_FALSE))
	{
		printf("Could not create window!\n");
		return -1;
	}

	glusRun();

	return 0;
}
