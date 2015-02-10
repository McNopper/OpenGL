/**
 * OpenGL 3 - Example 12
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
 * Width of the viewport, needed to reset from framebuffer.
 */
static GLUSuint g_width;

/**
 * Height of the viewport.
 */
static GLUSuint g_height;

static GLUSfloat g_cameraPosition[3] = { -5.0f, 5.0f, 10.0f };

static GLUSfloat g_lightPosition[3] = { 0.0f, 0.0f, 10.0f };

/**
 * Matrix, to convert from scene world space to shadow projection space.
 */
static GLfloat g_shadowMatrix[16];

static GLUSprogram g_program;

static GLint g_shadowMatrixLocation;

static GLint g_projectionMatrixLocation;

static GLint g_viewMatrixLocation;

static GLint g_modelMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_shadowTextureLocation;

static GLint g_lightDirectionLocation;

static GLint g_colorLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

//

static GLUSprogram g_programShadow;

static GLint g_projectionMatrixShadowLocation;

static GLint g_modelViewMatrixShadowLocation;

static GLint g_vertexShadowLocation;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_indicesVBO;

static GLuint g_verticesBackgroundVBO;

static GLuint g_normalsBackgroundVBO;

static GLuint g_indicesBackgroundVBO;

static GLuint g_vao;

static GLuint g_vaoBackground;

static GLuint g_vaoShadow;

static GLuint g_vaoShadowBackground;

//

static GLuint g_numberIndicesSphere;

static GLuint g_numberIndicesBackground;

//

/**
 * The shadow texture size.
 */
static const GLuint g_shadowTextureSize = 1024;

/**
 * The shadow texture.
 */
static GLuint g_shadowTexture;

/**
 * The frame buffer object for the shadow texture.
 */
static GLuint g_fbo;

GLUSboolean init(GLUSvoid)
{
    GLUSshape background;

    GLUSshape torus;

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLfloat viewMatrix[16];

    GLfloat lightDirection[3];

    lightDirection[0] = g_lightPosition[0];
    lightDirection[1] = g_lightPosition[1];
    lightDirection[2] = g_lightPosition[2];

    glusVector3Normalizef(lightDirection);

    //

    glusFileLoadText("../Example12/shader/rendershadow.vert.glsl", &vertexSource);
    glusFileLoadText("../Example12/shader/rendershadow.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_programShadow, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    glusFileLoadText("../Example12/shader/useshadow.vert.glsl", &vertexSource);
    glusFileLoadText("../Example12/shader/useshadow.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixShadowLocation = glGetUniformLocation(g_programShadow.program, "u_projectionMatrix");
    g_modelViewMatrixShadowLocation = glGetUniformLocation(g_programShadow.program, "u_modelViewMatrix");
    g_vertexShadowLocation = glGetAttribLocation(g_programShadow.program, "a_vertex");

    //

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_viewMatrixLocation = glGetUniformLocation(g_program.program, "u_viewMatrix");
    g_modelMatrixLocation = glGetUniformLocation(g_program.program, "u_modelMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_shadowMatrixLocation = glGetUniformLocation(g_program.program, "u_shadowMatrix");
    g_shadowTextureLocation = glGetUniformLocation(g_program.program, "u_shadowTexture");
    g_colorLocation = glGetUniformLocation(g_program.program, "u_shapeColor");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    glGenTextures(1, &g_shadowTexture);
    glBindTexture(GL_TEXTURE_2D, g_shadowTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, g_shadowTextureSize, g_shadowTextureSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);

    glBindTexture(GL_TEXTURE_2D, 0);

    //

    glGenFramebuffers(1, &g_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_shadowTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

        return GLUS_FALSE;
    }

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //

    glusShapeCreateTorusf(&torus, 0.5f, 1.0f, 32, 32);
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

    glUniform1i(g_shadowTextureLocation, 0);

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

    // Plane

    glGenVertexArrays(1, &g_vaoShadowBackground);
    glBindVertexArray(g_vaoShadowBackground);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
    glVertexAttribPointer(g_vertexShadowLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexShadowLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    // Needed when rendering the shadow map. This will avoid artifacts.
    glPolygonOffset(1.0f, 0.0f);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    static GLfloat biasMatrix[] = { 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f };

    GLfloat projectionMatrix[16];

    g_width = width;

    g_height = height;

    //

    glUseProgram(g_programShadow.program);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) g_shadowTextureSize / (GLfloat) g_shadowTextureSize, 1.0f, 100.0f);

    glUniformMatrix4fv(g_projectionMatrixShadowLocation, 1, GL_FALSE, projectionMatrix);

    glusMatrix4x4Identityf(g_shadowMatrix);
    glusMatrix4x4Multiplyf(g_shadowMatrix, g_shadowMatrix, biasMatrix);
    glusMatrix4x4Multiplyf(g_shadowMatrix, g_shadowMatrix, projectionMatrix);

    //

    glUseProgram(g_program.program);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
    static GLfloat angle = 0.0f;

    GLfloat shadowMatrix[16];
    GLfloat modelViewMatrix[16];
    GLfloat viewMatrix[16];
    GLfloat modelMatrix[16];
    GLfloat normalMatrix[9];

    // Rendering into the shadow texture.

    glBindTexture(GL_TEXTURE_2D, 0);

    // Setup for the framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
    glViewport(0, 0, g_shadowTextureSize, g_shadowTextureSize);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glusMatrix4x4LookAtf(viewMatrix, g_lightPosition[0], g_lightPosition[1], g_lightPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glusMatrix4x4Multiplyf(shadowMatrix, g_shadowMatrix, viewMatrix);

    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_programShadow.program);

    // Render the torus.

    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4RotateRzRxRyf(modelMatrix, 0.0f, 0.0f, angle);
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);

    glUniformMatrix4fv(g_modelViewMatrixShadowLocation, 1, GL_FALSE, modelViewMatrix);

    glBindVertexArray(g_vaoShadow);

    glEnable(GL_POLYGON_OFFSET_FILL);
    glFrontFace(GL_CW);

    glDrawElements(GL_TRIANGLES, g_numberIndicesSphere, GL_UNSIGNED_INT, 0);

    glDisable(GL_POLYGON_OFFSET_FILL);
    glFrontFace(GL_CCW);

    // Revert for the scene.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glViewport(0, 0, g_width, g_height);

    glBindTexture(GL_TEXTURE_2D, g_shadowTexture);

    //

    // Render the scene.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_program.program);

    glusMatrix4x4LookAtf(viewMatrix, g_cameraPosition[0], g_cameraPosition[1], g_cameraPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glUniformMatrix4fv(g_viewMatrixLocation, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(g_shadowMatrixLocation, 1, GL_FALSE, shadowMatrix);

    // Plane
    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4Translatef(modelMatrix, 0.0f, 0.0f, -5.0f);
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);
    glUniform4f(g_colorLocation, 0.0f, 0.5f, 0.0f, 1.0f);

    glBindVertexArray(g_vaoBackground);
    glDrawElements(GL_TRIANGLES, g_numberIndicesBackground, GL_UNSIGNED_INT, 0);

    // Torus
    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4RotateRzRxRyf(modelMatrix, 0.0f, 0.0f, angle);
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);
    glUniform4f(g_colorLocation, 0.33f, 0.0f, 0.5f, 1.0f);

    glBindVertexArray(g_vao);
    glDrawElements(GL_TRIANGLES, g_numberIndicesSphere, GL_UNSIGNED_INT, 0);

    //

    angle += 20.0f * time;

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glBindTexture(GL_TEXTURE_2D, 0);

    if (g_shadowTexture)
    {
        glDeleteRenderbuffers(1, &g_shadowTexture);

        g_shadowTexture = 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (g_fbo)
    {
        glDeleteFramebuffers(1, &g_fbo);

        g_fbo = 0;
    }

    //

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

    if (g_vaoShadowBackground)
    {
        glDeleteVertexArrays(1, &g_vaoShadowBackground);

        g_vaoShadowBackground = 0;
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
