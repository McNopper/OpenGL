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
#define LENGTH	500.0f

// Number of vertices on one side of the ocean grid. Has to be power of two.
#define N	512

#define GRAVITY 9.81f

#define AMPLITUDE 2.0f
#define WIND_SPEED 4.0f

static GLUSshaderprogram g_computeProgram;

static GLint g_processColumnLocation;

static GLint g_deltaTimeLocation;

//

static GLuint g_textureH0;
static GLuint g_textureH;
static GLuint g_textureDisplacement[2];

//

static GLUSshaderprogram g_program;

static GLint g_modelViewProjectionMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_colorLocation;

static GLint g_vertexLocation;

static GLint g_texCoordLocation;

static GLuint g_verticesVBO;

static GLuint g_texCoordsVBO;

static GLuint g_indicesVBO;

static GLuint g_vao;

//

static GLuint g_numberIndices;

//

static GLfloat phillipsSpectrum(GLfloat A, GLfloat L, GLfloat waveDirection[2], GLfloat windDirection[2])
{
	GLfloat k = glusVector2Lengthf(waveDirection);
	GLfloat waveDotWind = glusVector2Dotf(waveDirection, windDirection);

	return A * expf(-1.0f / (k * L * k * L)) / (k * k * k * k) * waveDotWind * waveDotWind;
}

GLUSboolean init(GLUSvoid)
{
    GLfloat lightDirection[3] = { 1.0f, 1.0f, 1.0f };
    GLfloat color[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

    GLUStextfile computeSource;

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape gridPlane;

    GLint i, k;
    GLfloat matrix[16];

    GLfloat* h0Data;

    GLfloat L = WIND_SPEED * WIND_SPEED / GRAVITY;

    GLfloat waveDirection[2];
    GLfloat windDirection[2] = {1.0f, 1.0f};
    GLfloat phillipsSpectrumValue;

    // TODO Load compute shader for H calculation.


    glusLoadTextFile("../Example41/shader/ocean_fft.comp.glsl", &computeSource);

    glusBuildComputeProgramFromSource(&g_computeProgram, (const GLchar**) &computeSource.text);

    glusDestroyTextFile(&computeSource);


    glusLoadTextFile("../Example41/shader/ocean.vert.glsl", &vertexSource);
    glusLoadTextFile("../Example41/shader/ocean.frag.glsl", &fragmentSource);

    glusBuildProgramFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusDestroyTextFile(&vertexSource);
    glusDestroyTextFile(&fragmentSource);

    //

    g_processColumnLocation = glGetUniformLocation(g_computeProgram.program, "u_processColumn");

    g_deltaTimeLocation = glGetUniformLocation(g_computeProgram.program, "u_deltaTime");


    g_modelViewProjectionMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewProjectionMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");
    g_colorLocation = glGetUniformLocation(g_program.program, "u_color");

	g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
	g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");

    //

    // Use a helper function to create a grid plane.
    glusCreateRectangularGridPlanef(&gridPlane, LENGTH, LENGTH, N - 1, N - 1, GLUS_FALSE);

    g_numberIndices = gridPlane.numberIndices;

    // Rotate by 90 degrees, that the grid is in the x-z-plane.
    glusMatrix4x4Identityf(matrix);
    glusMatrix4x4RotateRxf(matrix, -90.0f);
    for (i = 0; i < gridPlane.numberVertices; i++)
    {
    	glusMatrix4x4MultiplyPoint4f(&gridPlane.vertices[4 * i], matrix, &gridPlane.vertices[4 * i]);
    }

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, gridPlane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) gridPlane.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_texCoordsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER, gridPlane.numberVertices * 2 * sizeof(GLfloat), (GLfloat*) gridPlane.texCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridPlane.numberIndices * sizeof(GLuint), (GLuint*) gridPlane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusDestroyShapef(&gridPlane);

    //

    //
    // Generate H0.
    //

    h0Data = (GLfloat*)malloc(N * N * 2 * sizeof(GLfloat));

    if (!h0Data)
    {
    	return GLUS_FALSE;
    }

    glusVector2Normalizef(windDirection);

    for (i = 0; i < N; i++)
    {
    	// Positive N, that it matches with OpenGL z-axis.
    	waveDirection[1] = ((GLfloat)N / 2.0f - (GLfloat)i) * (2.0f * GLUS_PI / LENGTH);

        for (k = 0; k < N; k++)
        {
        	waveDirection[0] = ((GLfloat)-N / 2.0f + (GLfloat)k) * (2.0f * GLUS_PI / LENGTH);

        	phillipsSpectrumValue = phillipsSpectrum(AMPLITUDE, L, waveDirection, windDirection);

        	h0Data[i * 2 * N + k * 2 + 0] = 1.0f / sqrtf(2.0f) * glusRandomNormalGetFloatf(0.0f, 1.0f) * phillipsSpectrumValue;
        	h0Data[i * 2 * N + k * 2 + 1] = 1.0f / sqrtf(2.0f) * glusRandomNormalGetFloatf(0.0f, 1.0f) * phillipsSpectrumValue;
        }
    }

    //

    // TODO Generate indices needed for inverse FFT butterfly algorithm.

    //

    glGenTextures(1, &g_textureH0);
    glBindTexture(GL_TEXTURE_2D, g_textureH0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, N, N, 0, GL_RG, GL_FLOAT, h0Data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glGenTextures(1, &g_textureH);
    glBindTexture(GL_TEXTURE_2D, g_textureH);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, N, N, 0, GL_RG, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glGenTextures(1, &g_textureDisplacement[0]);
    glBindTexture(GL_TEXTURE_2D, g_textureDisplacement[0]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, N, N, 0, GL_RG, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glGenTextures(1, &g_textureDisplacement[1]);
    glBindTexture(GL_TEXTURE_2D, g_textureDisplacement[1]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, N, N, 0, GL_RG, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    glBindTexture(GL_TEXTURE_2D, 0);

    free(h0Data);

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

    //
    // Update H pass.
    //

    // TODO Calculate H depending on time.


    glUseProgram(g_computeProgram.program);

    glUniform1f(g_deltaTimeLocation, time);

    //
    // FFT per row pass.
    //

    glBindImageTexture(0, g_textureH, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, g_textureDisplacement[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

    glUniform1i(g_processColumnLocation, 0);

    // Process all vertices. N threads as N rows are processed. One work group is one row.
    glDispatchCompute(1, N, 1);

    // Make sure, all vertices are written.
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    //
    // FFT per column pass.
    //

    glBindImageTexture(0, g_textureDisplacement[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, g_textureDisplacement[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

    glUniform1i(g_processColumnLocation, 1);

    // Process all vertices. N threads as N columns are processed. One work group is one column.
    glDispatchCompute(1, N, 1);

    // Make sure, all vertices are written.
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);


    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, 0, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

    //
    // Drawing part.
    //

    glUseProgram(g_program.program);

    glBindVertexArray(g_vao);

    // TODO Revert, just for debugging.
    //glBindTexture(GL_TEXTURE_2D, g_textureDisplacement[1]);
    glBindTexture(GL_TEXTURE_2D, g_textureH0);

    glDrawElements(GL_TRIANGLES, g_numberIndices, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(0);

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glBindTexture(GL_TEXTURE_2D, 0);

    if (g_textureH0)
    {
        glDeleteTextures(1, &g_textureH0);

        g_textureH0 = 0;
    }

    if (g_textureH)
    {
        glDeleteTextures(1, &g_textureH);

        g_textureH = 0;
    }

    if (g_textureDisplacement[0])
    {
        glDeleteTextures(1, &g_textureDisplacement[0]);

        g_textureDisplacement[0] = 0;
    }

    if (g_textureDisplacement[1])
    {
        glDeleteTextures(1, &g_textureDisplacement[1]);

        g_textureDisplacement[1] = 0;
    }

	//

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

    //

    glUseProgram(0);

    glusDestroyProgram(&g_computeProgram);

    glusDestroyProgram(&g_program);
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
