/**
 * OpenGL 3 - Example 15
 *
 * @author  Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include "GL/glus.h"

#include "globals.h"
#include "renderBackground.h"

static GLUSprogram g_programBackground;

static GLint g_projectionMatrixBackgroundLocation;

static GLint g_modelViewMatrixBackgroundLocation;

static GLint g_vertexBackgroundLocation;

static GLint g_normalBackgroundLocation;

static GLint g_cubemapBackgroundLocation;

//

static GLuint g_vaoBackground;

static GLuint g_verticesBackgroundVBO;

static GLuint g_normalsBackgroundVBO;

static GLuint g_indicesBackgroundVBO;

static GLuint g_numberIndicesBackground;

GLUSboolean initBackground()
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape background;

    glusFileLoadText("../Example15/shader/Background.vert.glsl", &vertexSource);
    glusFileLoadText("../Example15/shader/Background.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_programBackground, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixBackgroundLocation = glGetUniformLocation(g_programBackground.program, "u_projectionMatrix");
    g_modelViewMatrixBackgroundLocation = glGetUniformLocation(g_programBackground.program, "u_modelViewMatrix");

    g_cubemapBackgroundLocation = glGetUniformLocation(g_programBackground.program, "u_cubemap");

    g_vertexBackgroundLocation = glGetAttribLocation(g_programBackground.program, "a_vertex");
    g_normalBackgroundLocation = glGetAttribLocation(g_programBackground.program, "a_normal");

    //

    glBindVertexArray(0);

    //

    glusShapeCreateSpheref(&background, (GLfloat) (GLfloat) WATER_PLANE_LENGTH / 2.0f + 0.5f, 32);
    g_numberIndicesBackground = background.numberIndices;

    glGenBuffers(1, &g_verticesBackgroundVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, background.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) background.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsBackgroundVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsBackgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, background.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) background.normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesBackgroundVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, background.numberIndices * sizeof(GLuint), (GLuint*) background.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&background);

    //

    glUseProgram(g_programBackground.program);

    // We assume, that the parent program created the texture!
    glUniform1i(g_cubemapBackgroundLocation, 0);

    //

    glGenVertexArrays(1, &g_vaoBackground);
    glBindVertexArray(g_vaoBackground);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
    glVertexAttribPointer(g_vertexBackgroundLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexBackgroundLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsBackgroundVBO);
    glVertexAttribPointer(g_normalBackgroundLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_normalBackgroundLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);

    return GLUS_TRUE;
}

GLUSvoid reshapeBackground(GLUSfloat projectionMatrix[16])
{
    glUseProgram(g_programBackground.program);

    glUniformMatrix4fv(g_projectionMatrixBackgroundLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean renderBackground(GLUSfloat viewMatrix[16])
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_programBackground.program);

    glUniformMatrix4fv(g_modelViewMatrixBackgroundLocation, 1, GL_FALSE, viewMatrix);

    glBindVertexArray(g_vaoBackground);

    glFrontFace(GL_CW);

    glDrawElements(GL_TRIANGLES, g_numberIndicesBackground, GL_UNSIGNED_INT, 0);

    return GLUS_TRUE;
}

GLUSvoid terminateBackground(GLUSvoid)
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (g_verticesBackgroundVBO)
    {
        glDeleteBuffers(1, &g_verticesBackgroundVBO);

        g_verticesBackgroundVBO = 0;
    }

    if (g_normalsBackgroundVBO)
    {
        glDeleteBuffers(1, &g_normalsBackgroundVBO);

        g_normalsBackgroundVBO = 0;
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

    glusProgramDestroy(&g_programBackground);
}
