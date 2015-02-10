/**
 * OpenGL 3 - Example 10
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

static GLint g_viewMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightPositionLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

//

static GLfloat g_projectionMatrix[16];

static GLfloat g_viewMatrix[16];

static GLfloat g_normalMatrix[9];

static GLfloat g_lightPosition[4] = {0.0f, 0.5f, 2.0f, 1.0};

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_indicesVBO;

static GLuint g_vao;

//

static GLuint g_numberIndicesSphere;

GLUSboolean init(GLUSvoid)
{
    GLUSshape sphere;

    GLUStextfile vertexSource;
    GLUStextfile geometrySource;
    GLUStextfile fragmentSource;

    glusFileLoadText("../Example10/shader/dublicate.vert.glsl", &vertexSource);
    glusFileLoadText("../Example10/shader/dublicate.geom.glsl", &geometrySource);
    glusFileLoadText("../Example10/shader/dublicate.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, (const GLUSchar**) &geometrySource.text, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&geometrySource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_viewMatrixLocation = glGetUniformLocation(g_program.program, "u_viewMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_lightPositionLocation = glGetUniformLocation(g_program.program, "u_lightPosition");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    glusShapeCreateSpheref(&sphere, 1.0f, 32);
    g_numberIndicesSphere = sphere.numberIndices;

    glGenBuffers(1, &g_verticesVBO);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) sphere.vertices, GL_STATIC_DRAW);
    glGenBuffers(1, &g_normalsVBO);

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) sphere.normals, GL_STATIC_DRAW);
    glGenBuffers(1, &g_indicesVBO);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.numberIndices * sizeof(GLuint), (GLuint*) sphere.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&sphere);

    //

    glUseProgram(g_program.program);

    glusMatrix4x4LookAtf(g_viewMatrix, 0.0f, 0.0f, 6.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glUniformMatrix4fv(g_viewMatrixLocation, 1, GL_FALSE, g_viewMatrix);

    glusMatrix4x4ExtractMatrix3x3f(g_normalMatrix, g_viewMatrix);

    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, g_normalMatrix);

    glusMatrix4x4MultiplyPoint4f(g_lightPosition, g_viewMatrix, g_lightPosition);

    glUniform4fv(g_lightPositionLocation, 1, g_lightPosition);

    //

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_normalLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

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

    glusMatrix4x4Perspectivef(g_projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, g_projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, g_numberIndicesSphere, GL_UNSIGNED_INT, 0);

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

    if (g_normalsVBO)
    {
        glDeleteBuffers(1, &g_normalsVBO);

        g_normalsVBO = 0;
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
	        EGL_DEPTH_SIZE, 24,
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
