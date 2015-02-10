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

//
// Theory and References see:
// 2D FFT
// http://paulbourke.net/miscellaneous/dft/
//
// Simulating Ocean Waves/Water
// http://graphics.ucsd.edu/courses/rendering/2005/jdewall/tessendorf.pdf
//
// http://www.edxgraphics.com/blog/simulating-ocean-waves-with-fft-on-gpu
//
// https://developer.nvidia.com/sites/default/files/akamai/gamedev/files/sdk/11/OceanCS_Slides.pdf
//
// http://www.keithlantz.net/2011/10/ocean-simulation-part-one-using-the-discrete-fourier-transform/
// http://www.keithlantz.net/2011/11/ocean-simulation-part-two-using-the-fast-fourier-transform/
//

// Note: If N and LENGTH is changed, the values has to be changed in the shaders as well.

// Number of vertices on one side of the ocean grid. Has to be power of two.
#define N 512

// Length of the ocean grid.
#define LENGTH	250.0f

// Gravity.
#define GRAVITY 9.81f

// Amplitude of the wave.
#define AMPLITUDE 0.002f

// Wind speed.
#define WIND_SPEED 10.0f

// Wind direction.
#define WIND_DIRECTION {1.0f, 1.0f}

// Largest possible wave arising.
#define LPWA (WIND_SPEED * WIND_SPEED / GRAVITY)

//

static GLUSprogram g_computeUpdateHtProgram;

static GLint g_totalTimeUpdateHtLocation;


static GLUSprogram g_computeFftProgram;

static GLint g_processColumnFftLocation;

static GLint g_stepsFftLocation;


static GLUSprogram g_computeUpdateNormalProgram;

//

static GLuint g_textureH0;
static GLuint g_textureHt;
static GLuint g_textureIndices;
static GLuint g_textureDisplacement[2];
static GLuint g_textureNormal;

//

static GLUSprogram g_program;

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

static GLfloat phillipsSpectrum(GLfloat A, GLfloat L, GLfloat waveDirection[2], GLfloat windDirection[2])
{
	GLfloat k = glusVector2Lengthf(waveDirection);
	GLfloat waveDotWind = glusVector2Dotf(waveDirection, windDirection);

	// Avoid division by zero.
	if (L == 0.0f || k == 0.0f)
	{
		return 0.0f;
	}

	return A * expf(-1.0f / (k * L * k * L)) / (k * k * k * k) * waveDotWind * waveDotWind;
}

GLUSboolean init(GLUSvoid)
{
    GLfloat lightDirection[3] = { 1.0f, 1.0f, 1.0f };
    GLfloat color[4] = { 0.0f, 0.8f, 0.8f, 1.0f };

    GLUStextfile computeSource;

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape gridPlane;

    GLint i, k;
    GLfloat matrix[16];

    GLfloat* h0Data;

    GLint* butterflyIndices;
    GLfloat* butterflyIndicesAsFloat;

    GLfloat waveDirection[2];
    GLfloat windDirection[2] = WIND_DIRECTION;
    GLfloat phillipsSpectrumValue;

    //

	GLint steps = 0;
	GLint temp = N;

	while (!(temp & 0x1))
	{
		temp = temp >> 1;
		steps++;
	}

    //

    glusFileLoadText("../Example41/shader/ocean_update_ht.comp.glsl", &computeSource);

    glusProgramBuildComputeFromSource(&g_computeUpdateHtProgram, (const GLchar**) &computeSource.text);

    glusFileDestroyText(&computeSource);


    glusFileLoadText("../Example41/shader/ocean_fft.comp.glsl", &computeSource);

    glusProgramBuildComputeFromSource(&g_computeFftProgram, (const GLchar**) &computeSource.text);

    glusFileDestroyText(&computeSource);


    glusFileLoadText("../Example41/shader/ocean_update_normal.comp.glsl", &computeSource);

    glusProgramBuildComputeFromSource(&g_computeUpdateNormalProgram, (const GLchar**) &computeSource.text);

    glusFileDestroyText(&computeSource);


    glusFileLoadText("../Example41/shader/ocean.vert.glsl", &vertexSource);
    glusFileLoadText("../Example41/shader/ocean.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_totalTimeUpdateHtLocation = glGetUniformLocation(g_computeUpdateHtProgram.program, "u_totalTime");


    g_processColumnFftLocation = glGetUniformLocation(g_computeFftProgram.program, "u_processColumn");
    g_stepsFftLocation = glGetUniformLocation(g_computeFftProgram.program, "u_steps");


    g_modelViewProjectionMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewProjectionMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");
    g_colorLocation = glGetUniformLocation(g_program.program, "u_color");

	g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
	g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");

    //

    // Use a helper function to create a grid plane.
    glusShapeCreateRectangularGridPlanef(&gridPlane, LENGTH, LENGTH, N - 1, N - 1, GLUS_FALSE);

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

    glusShapeDestroyf(&gridPlane);

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

        	phillipsSpectrumValue = phillipsSpectrum(AMPLITUDE, LPWA, waveDirection, windDirection);

        	h0Data[i * 2 * N + k * 2 + 0] = 1.0f / sqrtf(2.0f) * glusRandomNormalf(0.0f, 1.0f) * phillipsSpectrumValue;
        	h0Data[i * 2 * N + k * 2 + 1] = 1.0f / sqrtf(2.0f) * glusRandomNormalf(0.0f, 1.0f) * phillipsSpectrumValue;
        }
    }

    //

    glGenTextures(1, &g_textureH0);
    glBindTexture(GL_TEXTURE_2D, g_textureH0);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, N, N, 0, GL_RG, GL_FLOAT, h0Data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    free(h0Data);


    glGenTextures(1, &g_textureHt);
    glBindTexture(GL_TEXTURE_2D, g_textureHt);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, N, N, 0, GL_RG, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glGenTextures(1, &g_textureDisplacement[0]);
    glBindTexture(GL_TEXTURE_2D, g_textureDisplacement[0]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, N, N, 0, GL_RG, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glGenTextures(1, &g_textureDisplacement[1]);
    glBindTexture(GL_TEXTURE_2D, g_textureDisplacement[1]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, N, N, 0, GL_RG, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glGenTextures(1, &g_textureNormal);
    glBindTexture(GL_TEXTURE_2D, g_textureNormal);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, N, N, 0, GL_RGBA, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    glBindTexture(GL_TEXTURE_2D, 0);

    //
    // Generate indices needed for inverse FFT butterfly algorithm.
    //

    butterflyIndices = (GLint*)malloc(N * sizeof(GLint));

    if (!butterflyIndices)
    {
    	return GLUS_FALSE;
    }

    butterflyIndicesAsFloat = (GLfloat*)malloc(N * sizeof(GLfloat));

    if (!butterflyIndicesAsFloat)
    {
    	free(butterflyIndices);

    	return GLUS_FALSE;
    }

    for (i = 0; i < N; i++)
    {
    	butterflyIndices[i] = i;
    }

    glusFourierButterflyShuffleFFTi(butterflyIndices, butterflyIndices, N);

    for (i = 0; i < N; i++)
    {
    	butterflyIndicesAsFloat[i] = (GLfloat)butterflyIndices[i];
    }

    free(butterflyIndices);

    glGenTextures(1, &g_textureIndices);
    glBindTexture(GL_TEXTURE_1D, g_textureIndices);

    // FIXME: On AMD hardware, an integer texture can not be created. So floats are used as a workaround.
    glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, N, 0, GL_RED, GL_FLOAT, butterflyIndicesAsFloat);

    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_1D, 0);

    free(butterflyIndicesAsFloat);

    //

    glUseProgram(g_computeFftProgram.program);

    glUniform1i(g_stepsFftLocation, steps);

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

    glusMatrix4x4Identityf(modelMatrix);

    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelMatrix);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 2000.0f);

    glusMatrix4x4LookAtf(viewMatrix, 0.0f, 5.0f, LENGTH / 2.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glusMatrix4x4Multiplyf(viewProjectionMatrix, projectionMatrix, viewMatrix);
    glusMatrix4x4Multiplyf(modelViewProjectionMatrix, viewProjectionMatrix, modelMatrix);

    glUniformMatrix4fv(g_modelViewProjectionMatrixLocation, 1, GL_FALSE, modelViewProjectionMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    glUseProgram(0);
}

GLUSboolean update(GLUSfloat time)
{
	static GLfloat totalTime = 0.0f;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //
    // Simulation part.
    //

    //
    // Update H pass.
    //

    glUseProgram(g_computeUpdateHtProgram.program);

    glBindImageTexture(0, g_textureH0, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, g_textureHt, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

    glUniform1f(g_totalTimeUpdateHtLocation, totalTime);

    // Process all vertices. No synchronization needed, so start NxN threads with local size of 1x1.
    glDispatchCompute(N, N, 1);

    // Make sure, all values are written.
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //
    // FFT pass.
    //

    glUseProgram(g_computeFftProgram.program);

    glBindImageTexture(2, g_textureIndices, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R32F);

    //
    // FFT per row pass.
    //

    glBindImageTexture(0, g_textureHt, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, g_textureDisplacement[0], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

    glUniform1i(g_processColumnFftLocation, 0);

    // Process all vertices. N groups as N rows are processed. One work group is one row.
    glDispatchCompute(1, N, 1);

    // Make sure, all values are written.
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //
    // FFT per column pass.
    //

    glBindImageTexture(0, g_textureDisplacement[0], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, g_textureDisplacement[1], 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RG32F);

    glUniform1i(g_processColumnFftLocation, 1);

    // Process all vertices. N groups as N columns are processed. One work group is one column.
    glDispatchCompute(1, N, 1);

    // Make sure, all values are written.
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //
    // Update normal map pass.
    //

    glUseProgram(g_computeUpdateNormalProgram.program);

    glBindImageTexture(0, g_textureDisplacement[1], 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    glBindImageTexture(1, g_textureNormal, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);

    // Process all vertices. No synchronization needed, so start NxN threads with local size of 1x1.
    glDispatchCompute(N, N, 1);

    // Make sure, all values are written.
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

    //
    // Drawing part.
    //

    glUseProgram(g_program.program);

    glBindVertexArray(g_vao);

    glBindTexture(GL_TEXTURE_2D, g_textureDisplacement[1]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_textureNormal);

    glDrawElements(GL_TRIANGLES, g_numberIndices, GL_UNSIGNED_INT, 0);

    glBindTexture(GL_TEXTURE_2D, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindVertexArray(0);

    //

    totalTime += time;

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glBindTexture(GL_TEXTURE_1D, 0);

    if (g_textureIndices)
    {
        glDeleteTextures(1, &g_textureIndices);

        g_textureIndices = 0;
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    if (g_textureH0)
    {
        glDeleteTextures(1, &g_textureH0);

        g_textureH0 = 0;
    }

    if (g_textureHt)
    {
        glDeleteTextures(1, &g_textureHt);

        g_textureHt = 0;
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

    if (g_textureNormal)
    {
        glDeleteTextures(1, &g_textureNormal);

        g_textureNormal = 0;
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

    glusProgramDestroy(&g_computeUpdateHtProgram);

    glusProgramDestroy(&g_computeFftProgram);

    glusProgramDestroy(&g_computeUpdateNormalProgram);

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
	        EGL_SAMPLE_BUFFERS, 1,
	        EGL_SAMPLES, 4,
	        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
	        EGL_NONE
	};

    EGLint eglContextAttributes[] = {
    		EGL_CONTEXT_MAJOR_VERSION, 4,
    		EGL_CONTEXT_MINOR_VERSION, 3,
    		EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
    		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    		EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
    		EGL_NONE
    };

    glusWindowSetInitFunc(init);

    glusWindowSetReshapeFunc(reshape);

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

    if (!glusWindowCreate("GLUS Example Window", 1024, 768, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
