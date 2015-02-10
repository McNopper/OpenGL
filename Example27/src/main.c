/**
 * OpenGL 3 - Example 27
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

static GLUSfloat g_cameraPosition[3] = { -5.0f, 5.0f, 10.0f };

static GLUSfloat g_lightDirection[3] = { 0.0f, 0.0f, 10.0f };

static GLUSprogram g_program;

static GLint g_projectionMatrixLocation;

static GLint g_viewMatrixLocation;

static GLint g_modelMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_colorLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

//

static GLUSprogram g_programShadow;

static GLint g_projectionMatrixShadowLocation;

static GLint g_viewMatrixShadowLocation;

static GLint g_shadowProjectionMatrixShadowLocation;

static GLint g_modelMatrixShadowLocation;

static GLint g_vertexShadowLocation;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_indicesVBO;

static GLuint g_verticesBackgroundVBO;

static GLuint g_normalsBackgroundVBO;

static GLuint g_indicesBackgroundVBO;

//

static GLuint g_vao;

static GLuint g_vaoShadow;

static GLuint g_vaoBackground;

//

static GLuint g_numberIndicesTorus;

static GLuint g_numberIndicesBackground;

GLUSboolean init(GLUSvoid)
{
    GLUSshape background;

    GLUSshape torus;

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLfloat viewMatrix[16];

    GLfloat lightDirection[3];

    lightDirection[0] = g_lightDirection[0];
    lightDirection[1] = g_lightDirection[1];
    lightDirection[2] = g_lightDirection[2];

    glusVector3Normalizef(lightDirection);

    //

    glusFileLoadText("../Example27/shader/shadow.vert.glsl", &vertexSource);
    glusFileLoadText("../Example27/shader/shadow.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_programShadow, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    glusFileLoadText("../Example27/shader/color.vert.glsl", &vertexSource);
    glusFileLoadText("../Example27/shader/color.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixShadowLocation = glGetUniformLocation(g_programShadow.program, "u_projectionMatrix");
    g_viewMatrixShadowLocation = glGetUniformLocation(g_programShadow.program, "u_viewMatrix");
    g_shadowProjectionMatrixShadowLocation = glGetUniformLocation(g_programShadow.program, "u_shadowProjectionMatrix");
    g_modelMatrixShadowLocation = glGetUniformLocation(g_programShadow.program, "u_modelMatrix");
    g_vertexShadowLocation = glGetAttribLocation(g_programShadow.program, "a_vertex");

    //

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_viewMatrixLocation = glGetUniformLocation(g_program.program, "u_viewMatrix");
    g_modelMatrixLocation = glGetUniformLocation(g_program.program, "u_modelMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_colorLocation = glGetUniformLocation(g_program.program, "u_shapeColor");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    glusShapeCreateTorusf(&torus, 0.5f, 1.0f, 32, 32);
    g_numberIndicesTorus = torus.numberIndices;

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

    glusShapeCreatePlanef(&background, 10.0f);
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

    glUseProgram(g_program.program);

    glusMatrix4x4LookAtf(viewMatrix, g_cameraPosition[0], g_cameraPosition[1], g_cameraPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glusMatrix4x4MultiplyVector3f(lightDirection, viewMatrix, lightDirection);

    glUniform3fv(g_lightDirectionLocation, 1, lightDirection);

    // Torus

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_normalLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

    // Plane

    glGenVertexArrays(1, &g_vaoBackground);
    glBindVertexArray(g_vaoBackground);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsBackgroundVBO);
    glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_normalLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);

    //

    glUseProgram(g_programShadow.program);

    // Torus

    glGenVertexArrays(1, &g_vaoShadow);
    glBindVertexArray(g_vaoShadow);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexShadowLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexShadowLocation);

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
    GLfloat projectionMatrix[16];

    //

    glUseProgram(g_program.program);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);

    //

    glUseProgram(g_programShadow.program);

    glUniformMatrix4fv(g_projectionMatrixShadowLocation, 1, GL_FALSE, projectionMatrix);

}

GLUSboolean update(GLUSfloat time)
{
    static GLfloat angle = 0.0f;

    GLfloat modelViewMatrix[16];

    GLfloat viewMatrix[16];
    GLfloat shadowProjectionMatrix[16];
    GLfloat modelMatrix[16];
    GLfloat normalMatrix[9];

    // This shadow plane represents mathematically the background plane
    GLfloat shadowPlane[4] = {0.0f, 0.0f, 1.0f, 5.0f};

    glusMatrix4x4LookAtf(viewMatrix, g_cameraPosition[0], g_cameraPosition[1], g_cameraPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //
    // Render the scene.
    //

    glUseProgram(g_program.program);

    glUniformMatrix4fv(g_viewMatrixLocation, 1, GL_FALSE, viewMatrix);

    // Background Plane

    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4Translatef(modelMatrix, 0.0f, 0.0f, -5.0f);
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);
    glUniform4f(g_colorLocation, 0.0f, 0.5f, 0.0f, 1.0f);

    glBindVertexArray(g_vaoBackground);
    glDrawElements(GL_TRIANGLES, g_numberIndicesBackground, GL_UNSIGNED_INT, 0);

    //
    // Render the planar shadow
    //

    glUseProgram(g_programShadow.program);

    glUniformMatrix4fv(g_viewMatrixShadowLocation, 1, GL_FALSE, viewMatrix);

    // Torus projected as a shadow

    glusMatrix4x4PlanarShadowDirectionalLightf(shadowProjectionMatrix, shadowPlane, g_lightDirection);
    glUniformMatrix4fv(g_shadowProjectionMatrixShadowLocation, 1, GL_FALSE, shadowProjectionMatrix);

    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4RotateRzRxRyf(modelMatrix, 0.0f, 0.0f, angle);
    glUniformMatrix4fv(g_modelMatrixShadowLocation, 1, GL_FALSE, modelMatrix);

    glBindVertexArray(g_vaoShadow);

    // Overwrite the background plane
    glDisable(GL_DEPTH_TEST);

    glDrawElements(GL_TRIANGLES, g_numberIndicesTorus, GL_UNSIGNED_INT, 0);

    glEnable(GL_DEPTH_TEST);

    // Torus with color

    glUseProgram(g_program.program);

    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);
    glUniform4f(g_colorLocation, 0.33f, 0.0f, 0.5f, 1.0f);

    glBindVertexArray(g_vao);
    glDrawElements(GL_TRIANGLES, g_numberIndicesTorus, GL_UNSIGNED_INT, 0);

    //

    angle += 20.0f * time;

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

    //

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

    if (g_indicesBackgroundVBO)
    {
        glDeleteBuffers(1, &g_indicesBackgroundVBO);

        g_indicesBackgroundVBO = 0;
    }

    if (g_vaoBackground)
    {
        glDeleteVertexArrays(1, &g_vaoBackground);

        g_vao = 0;
    }

    glUseProgram(0);

    glusProgramDestroy(&g_program);

    //

    if (g_vaoShadow)
    {
        glDeleteVertexArrays(1, &g_vaoShadow);

        g_vaoShadow = 0;
    }

    glusProgramDestroy(&g_programShadow);
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
