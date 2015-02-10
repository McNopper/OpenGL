/**
 * OpenGL 3 - Example 20
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

#include "wavefront.h"

static GLfloat g_viewMatrix[16];

static GLUSprogram g_program;

// The matrix locations in the program.

static GLint g_projectionMatrixLocation;

static GLint g_modelViewMatrixLocation;

static GLint g_textureMatrixLocation;

static GLint g_repeatLocation;

static GLint g_vertexLocation;

static GLint g_texCoordLocation;

//

static GLuint g_verticesVBO;

static GLuint g_texCoordsVBO;

static GLuint g_indicesVBO;

//

static GLuint g_vao;

//

static GLuint g_numberIndicesPlane;

static struct LightProperties g_light = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

static struct CameraProperties g_camera = { {0.0f, 0.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};

GLUSboolean init(GLUSvoid)
{
    GLfloat modelViewMatrix[16];

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape plane;

    //

    glusMatrix4x4LookAtf(g_viewMatrix, g_camera.eye[0], g_camera.eye[1], g_camera.eye[2], g_camera.center[0], g_camera.center[1], g_camera.center[2], g_camera.up[0], g_camera.up[1], g_camera.up[2]);

    if (!initWavefront(g_viewMatrix, &g_light))
    {
    	return GLUS_FALSE;
    }

    //

    glusFileLoadText("../Example20/shader/checker.vert.glsl", &vertexSource);
    glusFileLoadText("../Example20/shader/checker.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    // Retrieve the uniform locations in the program.
    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_modelViewMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewMatrix");
    g_textureMatrixLocation = glGetUniformLocation(g_program.program, "u_textureMatrix");
    g_repeatLocation =  glGetUniformLocation(g_program.program, "u_repeat");

    // Retrieve the attribute locations in the program.
    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");

    //

    glusShapeCreatePlanef(&plane, 2.0f);

    g_numberIndicesPlane = plane.numberIndices;

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) plane.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_texCoordsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 2 * sizeof(GLfloat), (GLfloat*) plane.texCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, plane.numberIndices * sizeof(GLuint), (GLuint*) plane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&plane);

    //

    glUseProgram(g_program.program);

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
    glVertexAttribPointer(g_texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_texCoordLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

    //

    glusMatrix4x4Identityf(modelViewMatrix);
    glusMatrix4x4Translatef(modelViewMatrix, 0.0f, 0.0f, 4.0f);
    glusMatrix4x4Multiplyf(modelViewMatrix, g_viewMatrix, modelViewMatrix);

    glUniformMatrix4fv(g_modelViewMatrixLocation, 1, GL_FALSE, modelViewMatrix);

    // This defines the checker pattern. One is blended, one is discarded.
    glUniform1f(g_repeatLocation, 16.0f);

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    // Basic blending equation.
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	GLfloat projectionMatrix[16];

    glViewport(0, 0, width, height);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    reshapeWavefront(projectionMatrix);

    //

    glUseProgram(g_program.program);

    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
    static GLfloat angle = 0.0f;

	GLfloat textureMatrix[4];

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!updateWavefront(time))
    {
    	return GLUS_FALSE;
    }

    // Rotate the texture coordinates, so the checker pattern rotates around the texture coordinate origin.
    // Please note, that the pattern rotates CW, as we are not rotating the plane but instead the texture coordinates CCW.
    // As the vertices stay at their positions and the texture coordinates "travel" CCW, the final picture rotates CW.
    glusMatrix2x2Identityf(textureMatrix);
    glusMatrix2x2Rotatef(textureMatrix, angle);

    glUseProgram(g_program.program);

    glUniformMatrix2fv(g_textureMatrixLocation, 1, GL_FALSE, textureMatrix);

    glBindVertexArray(g_vao);

    glDrawElements(GL_TRIANGLES, g_numberIndicesPlane, GL_UNSIGNED_INT, 0);

    angle += 15.0f * time;

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

    if (g_texCoordsVBO)
    {
        glDeleteBuffers(1, &g_texCoordsVBO);

        g_texCoordsVBO = 0;
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

    terminateWavefront();
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
