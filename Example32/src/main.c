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

static GLfloat g_projectionMatrix[16];

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

	glUseProgram(g_programBackground.program);

	glUniform1i(g_panoramaTextureBackgroundLocation, 0);

	glGenVertexArrays(1, &g_vaoBackground);
	glBindVertexArray(g_vaoBackground);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
	glVertexAttribPointer(g_vertexBackgroundLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_vertexBackgroundLocation);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);

	//

	glBindTexture(GL_TEXTURE_2D, g_panoramaTexture);

	//

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	glViewport(0, 0, width, height);

	glusPerspectivef(g_projectionMatrix, 60.0f, (GLfloat)width / (GLfloat)height, 1.0f, 1000.0f);
}

GLUSboolean update(GLUSfloat time)
{
	GLfloat viewProjectionMatrix[16];
	GLfloat viewMatrix[16];

	// TODO Adjust look at.
	glusLookAtf(viewMatrix, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, -1.0f, 0.0f, 1.0f, 0.0f);

	glusMatrix4x4Multiplyf(viewProjectionMatrix, g_projectionMatrix, viewMatrix);

	// TODO Render to FBO and do tone mapping and gamma correction.

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render the background.

	glUseProgram(g_programBackground.program);

	glUniformMatrix4fv(g_viewProjectionMatrixBackgroundLocation, 1, GL_FALSE, viewProjectionMatrix);

	glBindVertexArray(g_vaoBackground);

	glFrontFace(GL_CW);

	glDrawElements(GL_TRIANGLES, g_numberIndicesBackground, GL_UNSIGNED_INT, 0);

	glFrontFace(GL_CCW);

	// TODO Render model.

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
}

int main(int argc, char* argv[])
{
	glusInitFunc(init);

	glusReshapeFunc(reshape);

	glusUpdateFunc(update);

	glusTerminateFunc(terminate);

	glusPrepareContext(4, 1, GLUS_FORWARD_COMPATIBLE_BIT);

	if (!glusCreateWindow("GLUS Example Window", 640, 480, 24, 0, GLUS_FALSE))
	{
		printf("Could not create window!\n");
		return -1;
	}

	glusRun();

	return 0;
}
