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

static GLUSprogram g_program;

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

	glusFileLoadText("../Example13/shader/tessellation.vert.glsl", &vertexSource);
	glusFileLoadText("../Example13/shader/tessellation.cont.glsl", &controlSource);
	glusFileLoadText("../Example13/shader/tessellation.eval.glsl", &evaluationSource);
	glusFileLoadText("../Example13/shader/tessellation.geom.glsl", &geometrySource);
	glusFileLoadText("../Example13/shader/tessellation.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, (const GLUSchar**) &controlSource.text, (const GLUSchar**) &evaluationSource.text, (const GLUSchar**) &geometrySource.text, (const GLUSchar**) &fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&controlSource);
	glusFileDestroyText(&evaluationSource);
	glusFileDestroyText(&geometrySource);
	glusFileDestroyText(&fragmentSource);

	//

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_modelViewMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewMatrix");
    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");

	//

    glusShapeCreatePlanef(&plane, 1.0f);

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) plane.vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_numberIndicesSphere * sizeof(GLuint), (GLuint*) planeIndices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&plane);

    //

    glUseProgram(g_program.program);

    // Calculate the view matrix ...
    glusMatrix4x4LookAtf(modelViewMatrix, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

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

	glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

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

	glusProgramDestroy(&g_program);
}

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
    		EGL_CONTEXT_MAJOR_VERSION, 4,
    		EGL_CONTEXT_MINOR_VERSION, 1,
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
