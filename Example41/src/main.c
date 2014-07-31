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

#define BINDING_BUFFER_COMP_FFT_VERTICES_IN 1
#define BINDING_BUFFER_COMP_FFT_VERTICES_OUT 2

#define BINDING_BUFFER_COMP_NORMAL_VERTICES_IN 1
#define BINDING_BUFFER_COMP_NORMAL_NORMALS_OUT 2

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

static GLUSshaderprogram g_computeFftProgram;

static GLint g_xFactorLocation;
static GLint g_yFactorLocation;

static GLint g_deltaTimeLocation;

static GLUSshaderprogram g_computeNormalProgram;

//

static GLUSshaderprogram g_program;

static GLint g_modelViewProjectionMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_colorLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLuint g_indicesVBO;

static GLuint g_vao;

//

static GLuint g_verticesInBuffer;

static GLuint g_verticesTempBuffer;

static GLuint g_verticesOutBuffer;

static GLuint g_normalsOutBuffer;

//

static GLUSshape g_gridPlane;

GLUSboolean init(GLUSvoid)
{
    GLfloat lightDirection[3] = { 1.0f, 1.0f, 1.0f };
    GLfloat color[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

    GLUStextfile computeSource;

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLint i;
    GLfloat matrix[16];

    glusLoadTextFile("../Example41/shader/ocean_fft.comp.glsl", &computeSource);

    glusBuildComputeProgramFromSource(&g_computeFftProgram, (const GLchar**) &computeSource.text);

    glusDestroyTextFile(&computeSource);


    glusLoadTextFile("../Example41/shader/ocean_normal.comp.glsl", &computeSource);

    glusBuildComputeProgramFromSource(&g_computeNormalProgram, (const GLchar**) &computeSource.text);

    glusDestroyTextFile(&computeSource);


    glusLoadTextFile("../Example41/shader/ocean.vert.glsl", &vertexSource);
    glusLoadTextFile("../Example41/shader/ocean.frag.glsl", &fragmentSource);

    glusBuildProgramFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusDestroyTextFile(&vertexSource);
    glusDestroyTextFile(&fragmentSource);

    //

    g_xFactorLocation = glGetUniformLocation(g_computeFftProgram.program, "u_xFactor");
    g_yFactorLocation = glGetUniformLocation(g_computeFftProgram.program, "u_yFactor");

    g_deltaTimeLocation = glGetUniformLocation(g_computeFftProgram.program, "u_deltaTime");


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

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_gridPlane.numberIndices * sizeof(GLuint), (GLuint*) g_gridPlane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //

	glGenBuffers(1, &g_verticesInBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_verticesInBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), g_gridPlane.vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &g_verticesTempBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_verticesTempBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &g_verticesOutBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_verticesOutBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &g_normalsOutBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_normalsOutBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);

    //

    glUseProgram(g_program.program);

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesOutBuffer);
	glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_vertexLocation);

	glBindBuffer(GL_ARRAY_BUFFER, g_normalsOutBuffer);
	glVertexAttribPointer(g_normalLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
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

    glUseProgram(g_computeFftProgram.program);

    glUniform1f(g_deltaTimeLocation, time);

    //
    // FFT per row pass.
    //

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_FFT_VERTICES_IN, g_verticesInBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_FFT_VERTICES_OUT, g_verticesTempBuffer);

    // Setting index factors, that a row is processed. See shader code for more details.
    glUniform1i(g_xFactorLocation, 1);
    glUniform1i(g_yFactorLocation, N);

    // Process all vertices. N threads as N rows are processed. One work group is one row.
    glDispatchCompute(1, N, 1);

    // Make sure, all vertices are written.
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    //
    // FFT per column pass.
    //

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_FFT_VERTICES_IN, g_verticesTempBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_FFT_VERTICES_OUT, g_verticesOutBuffer);

    // Setting index factors, that a column is processed. See shader code for more details.
    glUniform1i(g_xFactorLocation, N);
    glUniform1i(g_yFactorLocation, 1);

    // Process all vertices. N threads as N columns are processed. One work group is one column.
    glDispatchCompute(1, N, 1);

    // Make sure, all vertices are written.
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);


    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_FFT_VERTICES_IN, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_FFT_VERTICES_OUT, 0);

    //
    // Calculating normals pass.
    //

    glUseProgram(g_computeNormalProgram.program);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_NORMAL_VERTICES_IN, g_verticesOutBuffer);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_NORMAL_NORMALS_OUT, g_normalsOutBuffer);

    // Process all vertices. Launch NxN threads, as now nothing has to be synchronized.
    glDispatchCompute(N, N, 1);

    // Make sure, all normals are written.
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_NORMAL_VERTICES_IN, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_NORMAL_NORMALS_OUT, 0);

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

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (g_verticesInBuffer)
	{
		glDeleteBuffers(1, &g_verticesInBuffer);

		g_verticesInBuffer = 0;
	}

	if (g_verticesTempBuffer)
	{
		glDeleteBuffers(1, &g_verticesTempBuffer);

		g_verticesTempBuffer = 0;
	}

	if (g_verticesOutBuffer)
	{
		glDeleteBuffers(1, &g_verticesOutBuffer);

		g_verticesOutBuffer = 0;
	}

	if (g_normalsOutBuffer)
	{
		glDeleteBuffers(1, &g_normalsOutBuffer);

		g_normalsOutBuffer = 0;
	}

    //

    glUseProgram(0);

    glusDestroyProgram(&g_computeFftProgram);

    glusDestroyProgram(&g_computeNormalProgram);

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
