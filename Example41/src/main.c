/**
 * OpenGL 4 - Example 41
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

// Theory and References see:
// 2D FFT
// http://paulbourke.net/miscellaneous/dft/
//
// Simulating Ocean Waves/Water
// http://www.edxgraphics.com/blog/simulating-ocean-waves-with-fft-on-gpu
// https://developer.nvidia.com/sites/default/files/akamai/gamedev/files/sdk/11/OceanCS_Slides.pdf
// http://graphics.ucsd.edu/courses/rendering/2005/jdewall/tessendorf.pdf

// Length of the ocean grid.
#define L	500.0f

// Number of vertices on one side of the ocean grid. Has to be power of two.
#define N	128

//

static GLUSshaderprogram g_program;

static GLint g_modelViewProjectionMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_colorLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_indicesVBO;

static GLuint g_vao;

//

static GLUSshape g_gridPlane;

GLUSboolean init(GLUSvoid)
{
    GLfloat lightDirection[3] = { 1.0f, 1.0f, 1.0f };
    GLfloat color[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLint i;
    GLfloat matrix[16];

    // TODO Load ocean compute shader.

    glusLoadTextFile("../Example41/shader/ocean.vert.glsl", &vertexSource);
    glusLoadTextFile("../Example41/shader/ocean.frag.glsl", &fragmentSource);

    glusBuildProgramFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusDestroyTextFile(&vertexSource);
    glusDestroyTextFile(&fragmentSource);

    //

    // TODO Gather locations in compute shader.

    g_modelViewProjectionMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewProjectionMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");
    g_colorLocation = glGetUniformLocation(g_program.program, "u_color");

	g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
	g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    // Use a helper function to create a grid plane.
    glusCreateRectangularGridPlanef(&g_gridPlane, L, L, N - 1, N - 1, GLUS_FALSE);

    // Rotate by 90 degrees, that the grid is in the x-z-plane.
    glusMatrix4x4Identityf(matrix);
    glusMatrix4x4RotateRxf(matrix, -90.0f);
    for (i = 0; i < g_gridPlane.numberVertices; i++)
    {
    	glusMatrix4x4MultiplyPoint4f(&g_gridPlane.vertices[4 * i], matrix, &g_gridPlane.vertices[4 * i]);
    }

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) g_gridPlane.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glBufferData(GL_ARRAY_BUFFER, g_gridPlane.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) g_gridPlane.normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_gridPlane.numberIndices * sizeof(GLuint), (GLuint*) g_gridPlane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //

    // TODO Create / use buffers for compute shader.

    //

    glUseProgram(g_program.program);

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
	glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_vertexLocation);

	glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
	glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_normalLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

    glBindVertexArray(0);

    //

    glusVector3Normalizef(lightDirection);
    glUniform3fv(g_lightDirectionLocation, 1, lightDirection);

    glUniform4fv(g_colorLocation, 1, color);

    glUseProgram(0);

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    GLfloat modelMatrix[16];
    GLfloat normalMatrix[9];
    GLfloat viewMatrix[16];
    GLfloat projectionMatrix[16];
    GLfloat viewProjectionMatrix[16];
    GLfloat modelViewProjectionMatrix[16];

    glUseProgram(g_program.program);

    glViewport(0, 0, width, height);

    // For now, the grid is just on the "ground".
    glusMatrix4x4Identityf(modelMatrix);

    // This model matrix is a rigid body transform. So no need for the inverse, transposed matrix.
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelMatrix);

    glusPerspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 1000.0f);

    glusLookAtf(viewMatrix, 0.0f, 400.0f, 700.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glusMatrix4x4Multiplyf(viewProjectionMatrix, projectionMatrix, viewMatrix);
    glusMatrix4x4Multiplyf(modelViewProjectionMatrix, viewProjectionMatrix, modelMatrix);

    glUniformMatrix4fv(g_modelViewProjectionMatrixLocation, 1, GL_FALSE, modelViewProjectionMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    glUseProgram(0);
}

GLUSboolean update(GLUSfloat time)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //
    // Simulation part.
    //

    // TODO Implement.

    //
    // Drawing part.
    //

    glUseProgram(g_program.program);

    glBindVertexArray(g_vao);

    glDrawElements(GL_TRIANGLES, g_gridPlane.numberIndices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

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

    glUseProgram(0);

    glusDestroyProgram(&g_program);

    //

    glusDestroyShapef(&g_gridPlane);
}

int main(int argc, char* argv[])
{
    glusInitFunc(init);

    glusReshapeFunc(reshape);

    glusUpdateFunc(update);

    glusTerminateFunc(terminate);

    glusPrepareContext(4, 3, GLUS_FORWARD_COMPATIBLE_BIT);

    glusPrepareMSAA(4);

    glusPrepareDebug(GLUS_TRUE);

    if (!glusCreateWindow("GLUS Example Window", 1024, 768, 24, 0, GLUS_FALSE))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusRun();

    return 0;
}
