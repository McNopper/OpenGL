/**
 * OpenGL 3 - Example 22
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

static GLUSprogram g_programShadowVolume;

static GLUSprogram g_programShadowPlane;

//

static GLint g_projectionMatrixLocation;

static GLint g_viewMatrixLocation;

static GLint g_modelMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_colorLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

//

static GLint g_projectionMatrixShadowVolumeLocation;

static GLint g_viewMatrixShadowVolumeLocation;

static GLint g_modelMatrixShadowVolumeLocation;

static GLint g_lightDirectionShadowVolumeLocation;

static GLint g_vertexShadowVolumeLocation;

//

static GLint g_vertexShadowPlaneLocation;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_indicesVBO;

static GLuint g_verticesPlaneVBO;

static GLuint g_normalsPlaneVBO;

static GLuint g_indicesPlaneVBO;

static GLuint g_verticesShadowPlaneVBO;

static GLuint g_indicesShadowPlaneVBO;

static GLuint g_vao;

static GLuint g_vaoShadowVolume;

static GLuint g_vaoPlane;

static GLuint g_vaoShadowPlane;

//

static GLuint g_numberIndices;

static GLuint g_numberIndicesPlane;

static GLuint g_numberIndicesShadowPlane;

GLUSboolean init(GLUSvoid)
{
	GLUSshape shadowPlane;

    GLUSshape plane;

    GLUSshape torus, torusWithAdjacency;

    GLUStextfile vertexSource;
    GLUStextfile geometrySource;
    GLUStextfile fragmentSource;

    GLfloat viewMatrix[16];

    GLfloat lightDirection[3];

    lightDirection[0] = g_lightDirection[0];
    lightDirection[1] = g_lightDirection[1];
    lightDirection[2] = g_lightDirection[2];

    glusVector3Normalizef(lightDirection);

    //

    glusFileLoadText("../Example22/shader/color.vert.glsl", &vertexSource);
    glusFileLoadText("../Example22/shader/color.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

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

    glusFileLoadText("../Example22/shader/shadowvolume.vert.glsl", &vertexSource);
    glusFileLoadText("../Example22/shader/shadowvolume.geom.glsl", &geometrySource);
    glusFileLoadText("../Example22/shader/shadowvolume.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_programShadowVolume, (const GLUSchar**) &vertexSource.text, 0, 0, (const GLUSchar**) &geometrySource.text, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&geometrySource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixShadowVolumeLocation = glGetUniformLocation(g_programShadowVolume.program, "u_projectionMatrix");
    g_viewMatrixShadowVolumeLocation = glGetUniformLocation(g_programShadowVolume.program, "u_viewMatrix");
    g_modelMatrixShadowVolumeLocation = glGetUniformLocation(g_programShadowVolume.program, "u_modelMatrix");
    g_lightDirectionShadowVolumeLocation = glGetUniformLocation(g_programShadowVolume.program, "u_lightDirection");

    g_vertexShadowVolumeLocation = glGetAttribLocation(g_programShadowVolume.program, "a_vertex");

    //

    glusFileLoadText("../Example22/shader/shadow.vert.glsl", &vertexSource);
    glusFileLoadText("../Example22/shader/shadow.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_programShadowPlane, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_vertexShadowPlaneLocation = glGetAttribLocation(g_programShadowPlane.program, "a_vertex");

    //

    glusShapeCreateTorusf(&torus, 0.5f, 1.0f, 32, 32);
    glusShapeCreateAdjacencyIndicesf(&torusWithAdjacency, &torus);
    glusShapeDestroyf(&torus);

    g_numberIndices = torusWithAdjacency.numberIndices;

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, torusWithAdjacency.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) torusWithAdjacency.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glBufferData(GL_ARRAY_BUFFER, torusWithAdjacency.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) torusWithAdjacency.normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, torusWithAdjacency.numberIndices * sizeof(GLuint), (GLuint*) torusWithAdjacency.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&torusWithAdjacency);

    //

    glusShapeCreatePlanef(&plane, 10.0f);
    g_numberIndicesPlane = plane.numberIndices;

    glGenBuffers(1, &g_verticesPlaneVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesPlaneVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) plane.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsPlaneVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsPlaneVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) plane.normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesPlaneVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesPlaneVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, plane.numberIndices * sizeof(GLuint), (GLuint*) plane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&plane);

    //

    // The plane extends from -1.0 to 1.0 for both sides. So when rendering in NDC, the plane is always fullscreen.
    glusShapeCreatePlanef(&shadowPlane, 1.0f);
    g_numberIndicesShadowPlane = shadowPlane.numberIndices;

    glGenBuffers(1, &g_verticesShadowPlaneVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesShadowPlaneVBO);
    glBufferData(GL_ARRAY_BUFFER, shadowPlane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) shadowPlane.vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesShadowPlaneVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesShadowPlaneVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, shadowPlane.numberIndices * sizeof(GLuint), (GLuint*) shadowPlane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&shadowPlane);

    //


    glusMatrix4x4LookAtf(viewMatrix, g_cameraPosition[0], g_cameraPosition[1], g_cameraPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // Bring light from world to camera / view space
    glusMatrix4x4MultiplyVector3f(lightDirection, viewMatrix, lightDirection);

    glUseProgram(g_program.program);

    glUniform3fv(g_lightDirectionLocation, 1, lightDirection);

    //

    glUseProgram(g_programShadowVolume.program);

    glUniform3fv(g_lightDirectionShadowVolumeLocation, 1, lightDirection);

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

    // Shadow Volume

    glGenVertexArrays(1, &g_vaoShadowVolume);
    glBindVertexArray(g_vaoShadowVolume);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexShadowVolumeLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexShadowVolumeLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

    // Plane

    glGenVertexArrays(1, &g_vaoPlane);
    glBindVertexArray(g_vaoPlane);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesPlaneVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsPlaneVBO);
    glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_normalLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesPlaneVBO);

    // Shadow Plane

    glGenVertexArrays(1, &g_vaoShadowPlane);
    glBindVertexArray(g_vaoShadowPlane);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesShadowPlaneVBO);
    glVertexAttribPointer(g_vertexShadowPlaneLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexShadowPlaneLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesShadowPlaneVBO);

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);

    glClearStencil(0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glPolygonOffset(0.0f, 100.0f);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    GLfloat projectionMatrix[16];

    glViewport(0, 0, width, height);

    //

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);


    glUseProgram(g_program.program);

    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);

    glUseProgram(g_programShadowVolume.program);

    glUniformMatrix4fv(g_projectionMatrixShadowVolumeLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
    static GLfloat angle = 0.0f;

    GLfloat modelViewMatrix[16];
    GLfloat viewMatrix[16];
    GLfloat modelMatrix[16];
    GLfloat normalMatrix[9];

    // Render the scene.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glUseProgram(g_program.program);

    glusMatrix4x4LookAtf(viewMatrix, g_cameraPosition[0], g_cameraPosition[1], g_cameraPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glUniformMatrix4fv(g_viewMatrixLocation, 1, GL_FALSE, viewMatrix);

    // Draw Color

    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Plane

    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4Translatef(modelMatrix, 0.0f, 0.0f, -5.0f);
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);
    glUniform4f(g_colorLocation, 0.0f, 0.5f, 0.0f, 1.0f);

    glBindVertexArray(g_vaoPlane);
    glDrawElements(GL_TRIANGLES, g_numberIndicesPlane, GL_UNSIGNED_INT, 0);

    // Torus

    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4RotateRzRxRyf(modelMatrix, 0.0f, 0.0f, angle);
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);
    glUniform4f(g_colorLocation, 0.33f, 0.0f, 0.5f, 1.0f);

    glBindVertexArray(g_vao);
    glDrawElements(GL_TRIANGLES_ADJACENCY, g_numberIndices, GL_UNSIGNED_INT, 0);

    // Draw Shadow Volume

    // Using zfail see http://joshbeam.com/articles/stenciled_shadow_volumes_in_opengl/
    glEnable(GL_STENCIL_TEST);

    glUseProgram(g_programShadowVolume.program);

    glUniformMatrix4fv(g_viewMatrixShadowVolumeLocation, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(g_modelMatrixShadowVolumeLocation, 1, GL_FALSE, modelMatrix);

    // Only render to the stencil buffer
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    // Avoid ugly artifacts
	glEnable(GL_POLYGON_OFFSET_FILL);
	// Needed, as vertices in the back are extruded to infinity
	glEnable(GL_DEPTH_CLAMP);

    glBindVertexArray(g_vaoShadowVolume);

    // Render the back faces ...
	glCullFace(GL_FRONT);
	glStencilFunc(GL_ALWAYS, 0x0, 0xff);
	glStencilOp(GL_KEEP, GL_INCR, GL_KEEP);
    glDrawElements(GL_TRIANGLES_ADJACENCY, g_numberIndices, GL_UNSIGNED_INT, 0);

    // ... and then the front faces
    glCullFace(GL_BACK);
	glStencilFunc(GL_ALWAYS, 0x0, 0xff);
	glStencilOp(GL_KEEP, GL_DECR, GL_KEEP);
    glDrawElements(GL_TRIANGLES_ADJACENCY, g_numberIndices, GL_UNSIGNED_INT, 0);

    // Reset
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glDisable(GL_DEPTH_CLAMP);

    // Draw shadow by blending a black, half transparent plane
    glUseProgram(g_programShadowPlane.program);

    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);

    // Only render, where the stencil buffer is not 0
    glStencilFunc(GL_NOTEQUAL, 0x0, 0xff);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);

    glBindVertexArray(g_vaoShadowPlane);
    glDrawElements(GL_TRIANGLES, g_numberIndicesShadowPlane, GL_UNSIGNED_INT, 0);

    glDisable(GL_BLEND);
    glDisable(GL_STENCIL_TEST);

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

    if (g_verticesPlaneVBO)
    {
        glDeleteBuffers(1, &g_verticesPlaneVBO);

        g_verticesPlaneVBO = 0;
    }

    if (g_normalsPlaneVBO)
    {
        glDeleteBuffers(1, &g_normalsPlaneVBO);

        g_normalsPlaneVBO = 0;
    }

    if (g_indicesPlaneVBO)
    {
        glDeleteBuffers(1, &g_indicesPlaneVBO);

        g_indicesPlaneVBO = 0;
    }

    if (g_vaoPlane)
    {
        glDeleteVertexArrays(1, &g_vaoPlane);

        g_vaoPlane = 0;
    }

    //

    if (g_verticesShadowPlaneVBO)
    {
        glDeleteBuffers(1, &g_verticesShadowPlaneVBO);

        g_verticesShadowPlaneVBO = 0;
    }

    if (g_indicesShadowPlaneVBO)
    {
        glDeleteBuffers(1, &g_indicesShadowPlaneVBO);

        g_indicesShadowPlaneVBO = 0;
    }

    if (g_vaoShadowPlane)
    {
        glDeleteVertexArrays(1, &g_vaoShadowPlane);

        g_vaoShadowPlane = 0;
    }

    glUseProgram(0);

    glusProgramDestroy(&g_program);

    glusProgramDestroy(&g_programShadowVolume);

    glusProgramDestroy(&g_programShadowPlane);
}

int main(int argc, char* argv[])
{
	EGLint eglConfigAttributes[] = {
	        EGL_RED_SIZE, 8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE, 8,
	        EGL_DEPTH_SIZE, 24,
	        EGL_STENCIL_SIZE, 8,
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
