/**
 * OpenGL 4 - Example 13
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

static GLUSshaderprogram g_program;

static GLint g_projectionMatrixLocation;

static GLint g_modelViewMatrixLocation;

static GLint g_vertexLocation;

//

static GLuint g_verticesVBO;

static GLuint g_indicesVBO;

//

static GLuint g_vao;

//

static const GLUSuint g_numberIndicesSphere = 4;

GLUSboolean init(GLUSvoid)
{
    GLfloat modelViewMatrix[16];

    GLUSshape plane;

    GLUSuint planeIndices[] = { 0, 1, 3, 2 };

	GLUStextfile vertexSource;
	GLUStextfile controlSource;
	GLUStextfile evaluationSource;
	GLUStextfile geometrySource;
	GLUStextfile fragmentSource;

	glusLoadTextFile("../Example13/shader/tessellation.vert.glsl", &vertexSource);
	glusLoadTextFile("../Example13/shader/tessellation.cont.glsl", &controlSource);
	glusLoadTextFile("../Example13/shader/tessellation.eval.glsl", &evaluationSource);
	glusLoadTextFile("../Example13/shader/tessellation.geom.glsl", &geometrySource);
	glusLoadTextFile("../Example13/shader/tessellation.frag.glsl", &fragmentSource);

	glusBuildProgramFromSource(&g_program, (const GLUSchar**) &vertexSource.text, (const GLUSchar**) &controlSource.text, (const GLUSchar**) &evaluationSource.text, (const GLUSchar**) &geometrySource.text, (const GLUSchar**) &fragmentSource.text);

	glusDestroyTextFile(&vertexSource);
	glusDestroyTextFile(&controlSource);
	glusDestroyTextFile(&evaluationSource);
	glusDestroyTextFile(&geometrySource);
	glusDestroyTextFile(&fragmentSource);

	//

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_modelViewMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewMatrix");
    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");

	//

    glusCreatePlanef(&plane, 1.0f);

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) plane.vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_numberIndicesSphere * sizeof(GLuint), (GLuint*) planeIndices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusDestroyShapef(&plane);

    //

    glUseProgram(g_program.program);

    // Calculate the view matrix ...
    glusLookAtf(modelViewMatrix, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glUniformMatrix4fv(g_modelViewMatrixLocation, 1, GL_FALSE, modelViewMatrix);

	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
	glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

	//

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable( GL_CULL_FACE);

	// We work with 4 points per patch.
    glPatchParameteri(GL_PATCH_VERTICES, 4);

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    GLfloat projectionMatrix[16];

	glViewport(0, 0, width, height);

	glusPerspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

	glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
	glClear(GL_COLOR_BUFFER_BIT);

	glDrawElements(GL_PATCHES, g_numberIndicesSphere, GL_UNSIGNED_INT, 0);

	return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (g_verticesVBO)
	{
		glDeleteBuffers(1, &g_verticesVBO);

		g_verticesVBO = 0;
	}

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (g_indicesVBO)
	{
		glDeleteBuffers(1, &g_indicesVBO);

		g_indicesVBO = 0;
	}

    glBindVertexArray(0);

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_vao);

        g_vao = 0;
    }

	glUseProgram(0);

	glusDestroyProgram(&g_program);
}

int main(int argc, char* argv[])
{
	glusInitFunc(init);

	glusReshapeFunc(reshape);

	glusUpdateFunc(update);

	glusTerminateFunc(terminate);

	glusPrepareContext(4, 1, GLUS_FORWARD_COMPATIBLE_BIT);

	if (!glusCreateWindow("GLUS Example Window", 640, 480, 0, 0, GLUS_FALSE))
	{
		printf("Could not create window!\n");
		return -1;
	}

	glusRun();

	return 0;
}
