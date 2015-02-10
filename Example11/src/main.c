/**
 * OpenGL 3 - Example 11
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include <math.h>

#include "GL/glus.h"

static GLUSprogram g_program;

static GLint g_viewProjectionMatrixLocation;

static GLint g_modelMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_cameraLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLint g_cubemapLocation;

static GLUSprogram g_programBackground;

static GLint g_viewProjectionMatrixBackgroundLocation;

static GLint g_modelMatrixBackgroundLocation;

static GLint g_vertexBackgroundLocation;

static GLint g_cubemapBackgroundLocation;

//

static GLfloat g_projectionMatrix[16];

//

static GLuint g_cubemap;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_indicesVBO;

static GLuint g_vao;

//

static GLuint g_verticesBackgroundVBO;

static GLuint g_indicesBackgroundVBO;

static GLuint g_vaoBackground;

//

static GLuint g_numberIndicesSphere;

static GLuint g_numberIndicesBackground;

//

static GLfloat g_circleRadius = 5.0f;

GLUSboolean init(GLUSvoid)
{
    GLUSshape torus;

    GLUSshape backgroundSphere;

    GLUStgaimage image;

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    glusFileLoadText("../Example11/shader/glass.vert.glsl", &vertexSource);
    glusFileLoadText("../Example11/shader/glass.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    glusFileLoadText("../Example11/shader/background.vert.glsl", &vertexSource);
    glusFileLoadText("../Example11/shader/background.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_programBackground, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_viewProjectionMatrixLocation = glGetUniformLocation(g_program.program, "u_viewProjectionMatrix");
    g_modelMatrixLocation = glGetUniformLocation(g_program.program, "u_modelMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_cameraLocation = glGetUniformLocation(g_program.program, "u_camera");
    g_cubemapLocation = glGetUniformLocation(g_program.program, "u_cubemap");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    g_viewProjectionMatrixBackgroundLocation = glGetUniformLocation(g_programBackground.program, "u_viewProjectionMatrix");
    g_modelMatrixBackgroundLocation = glGetUniformLocation(g_programBackground.program, "u_modelMatrix");
    g_cubemapBackgroundLocation = glGetUniformLocation(g_programBackground.program, "u_cubemap");

    g_vertexBackgroundLocation = glGetAttribLocation(g_programBackground.program, "a_vertex");

    //

    glGenTextures(1, &g_cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, g_cubemap);

    glusImageLoadTga("cm_pos_x.tga", &image);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);
    glusImageDestroyTga(&image);

    glusImageLoadTga("cm_neg_x.tga", &image);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);
    glusImageDestroyTga(&image);

    glusImageLoadTga("cm_pos_y.tga", &image);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);
    glusImageDestroyTga(&image);

    glusImageLoadTga("cm_neg_y.tga", &image);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);
    glusImageDestroyTga(&image);

    glusImageLoadTga("cm_pos_z.tga", &image);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);
    glusImageDestroyTga(&image);

    glusImageLoadTga("cm_neg_z.tga", &image);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);
    glusImageDestroyTga(&image);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    //

    glusShapeCreateTorusf(&torus, 0.25f, 1.0f, 32, 32);
    g_numberIndicesSphere = torus.numberIndices;

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, torus.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) torus.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glBufferData(GL_ARRAY_BUFFER, torus.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) torus.normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, torus.numberIndices * sizeof(GLuint), (GLuint*) torus.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&torus);

    //

    glusShapeCreateSpheref(&backgroundSphere, g_circleRadius, 32);
    g_numberIndicesBackground = backgroundSphere.numberIndices;

    glGenBuffers(1, &g_verticesBackgroundVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, backgroundSphere.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) backgroundSphere.vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesBackgroundVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, backgroundSphere.numberIndices * sizeof(GLuint), (GLuint*) backgroundSphere.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&backgroundSphere);

    //

    glUseProgram(g_program.program);

    glUniform1i(g_cubemapLocation, 0);

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

    glUseProgram(g_programBackground.program);

    glUniform1i(g_cubemapBackgroundLocation, 0);

    glGenVertexArrays(1, &g_vaoBackground);
    glBindVertexArray(g_vaoBackground);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
    glVertexAttribPointer(g_vertexBackgroundLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexBackgroundLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);

    //

    glBindTexture(GL_TEXTURE_CUBE_MAP, g_cubemap);

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
}

GLUSboolean update(GLUSfloat time)
{
    static GLfloat angle = 0.0f;

    GLfloat angleRadians;

    GLfloat viewProjectionMatrix[16];
    GLfloat viewMatrix[16];
    GLfloat modelMatrix[16];
    GLfloat normalMatrix[9];

    GLfloat camera[4] = {0.0, 0.0, 0.0, 1.0};

    angleRadians = glusMathDegToRadf(angle);

    camera[0] = g_circleRadius * -sinf(angleRadians);
    camera[2] = g_circleRadius * cosf(angleRadians);

    // Circle with the camera around the origin by looking at it.
    glusMatrix4x4LookAtf(viewMatrix, camera[0], 0.0f, camera[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glusMatrix4x4Multiplyf(viewProjectionMatrix, g_projectionMatrix, viewMatrix);

    glusMatrix4x4Identityf(modelMatrix);

    glusMatrix4x4Translatef(modelMatrix, 0.0f, -0.5f, 0.0f);
    glusMatrix4x4RotateRxf(modelMatrix, 45.0f);

    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelMatrix);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //

    // First render the background.

    glUseProgram(g_programBackground.program);

    glUniformMatrix4fv(g_viewProjectionMatrixBackgroundLocation, 1, GL_FALSE, viewProjectionMatrix);

    glUniformMatrix4fv(g_modelMatrixBackgroundLocation, 1, GL_FALSE, modelMatrix);

    glBindVertexArray(g_vaoBackground);

    glFrontFace(GL_CW);

    glDrawElements(GL_TRIANGLES, g_numberIndicesBackground, GL_UNSIGNED_INT, 0);

    // Now render the sphere.

    glUseProgram(g_program.program);

    glUniformMatrix4fv(g_viewProjectionMatrixLocation, 1, GL_FALSE, viewProjectionMatrix);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);

    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    glUniform4fv(g_cameraLocation, 1, camera);

    glBindVertexArray(g_vao);

    glFrontFace(GL_CCW);

    glDrawElements(GL_TRIANGLES, g_numberIndicesSphere, GL_UNSIGNED_INT, 0);

    // Increase the angle 30 degree per second.
    angle += 30.0f * time;

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    if (g_cubemap)
    {
        glDeleteTextures(1, &g_cubemap);

        g_cubemap = 0;
    }

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

    //

    if (g_verticesBackgroundVBO)
    {
        glDeleteBuffers(1, &g_verticesBackgroundVBO);

        g_verticesBackgroundVBO = 0;
    }

    if (g_indicesBackgroundVBO)
    {
        glDeleteBuffers(1, &g_indicesBackgroundVBO);

        g_indicesBackgroundVBO = 0;
    }

    if (g_vaoBackground)
    {
        glDeleteVertexArrays(1, &g_vaoBackground);

        g_vaoBackground = 0;
    }

    glusProgramDestroy(&g_programBackground);
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
