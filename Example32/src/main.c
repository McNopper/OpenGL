/**
 * OpenGL 4 - Example 32
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <math.h>
#include <stdio.h>

#include "GL/glus.h"

#define SCREEN_WIDTH	1024
#define SCREEN_HEIGHT	768
#define MSAA_SAMPLES	4

// Not more than 2^9 = 512 samples.
#define MAX_M	9

static GLfloat g_exposure = 3.0f;
static GLfloat g_gamma = 2.2f;

// Start with 2^6 = 64 samples.
static GLuint g_m = 6;

// Roughness of the material.
static GLfloat g_roughness = 0.1f;

// Reflection coefficient see http://en.wikipedia.org/wiki/Schlick%27s_approximation
static GLfloat g_R0 = 0.2f;

//

static GLfloat g_eye[4] = { 0.0f, 2.5f, 6.0f, 1.0f };

//

static GLfloat g_viewProjectionMatrix[16];

//

static GLUSprogram g_fullscreenProgram;

static GLint g_framebufferTextureFullscreenLocation;

static GLint g_msaaSamplesFullscreenLocation;
static GLint g_exposureFullscreenLocation;
static GLint g_gammaFullscreenLocation;

//

static GLuint g_fullscreenTexture;

static GLuint g_fullscreenDepthRenderbuffer;

static GLuint g_fullscreenFBO;

static GLuint g_fullscreenVAO;

//
//
//

static GLuint g_panoramaTexture;

//
//
//

static GLUSprogram g_modelProgram;

static GLint g_viewProjectionMatrixModelLocation;

static GLint g_modelMatrixModelLocation;

static GLint g_normalMatrixModelLocation;

static GLint g_eyeModelLocation;

static GLint g_panoramaTextureModelLocation;

static GLint g_colorMaterialModelLocation;
static GLint g_roughnessMaterialModelLocation;
static GLint g_R0MaterialModelLocation;

static GLint g_numberSamplesModelLocation;
static GLint g_mModelLocation;
static GLint g_binaryFractionFactorModelLocation;

static GLint g_vertexModelLocation;

static GLint g_normalModelLocation;

//

static GLuint g_verticesModelVBO;

static GLuint g_normalsModelVBO;

static GLuint g_modelVAO;

//

static GLuint g_numberVerticesModel;

//
//

static GLUSprogram g_backgroundProgram;

static GLint g_viewProjectionMatrixBackgroundLocation;

static GLint g_vertexBackgroundLocation;

static GLint g_panoramaTextureBackgroundLocation;

//

static GLuint g_verticesBackgroundVBO;

static GLuint g_indicesBackgroundVBO;

static GLuint g_backgroundVAO;

//

static GLuint g_numberIndicesBackground;

GLUSboolean init(GLUSvoid)
{
	GLUSshape backgroundSphere;

	GLUSshape wavefront;

	GLUShdrimage image;

	GLUStextfile vertexSource;
	GLUStextfile fragmentSource;

	GLfloat colorMaterial[3] = { 0.8, 0.8, 0.8 };

	glusFileLoadText("../Example32/shader/brdf.vert.glsl", &vertexSource);
	glusFileLoadText("../Example32/shader/brdf.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_modelProgram, (const GLchar**)&vertexSource.text, 0, 0, 0, (const GLchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	g_viewProjectionMatrixModelLocation = glGetUniformLocation(g_modelProgram.program, "u_viewProjectionMatrix");
	g_modelMatrixModelLocation = glGetUniformLocation(g_modelProgram.program, "u_modelMatrix");
	g_normalMatrixModelLocation = glGetUniformLocation(g_modelProgram.program, "u_normalMatrix");
	g_eyeModelLocation = glGetUniformLocation(g_modelProgram.program, "u_eye");
	g_panoramaTextureModelLocation = glGetUniformLocation(g_modelProgram.program, "u_panoramaTexture");
	g_colorMaterialModelLocation = glGetUniformLocation(g_modelProgram.program, "u_colorMaterial");
	g_roughnessMaterialModelLocation = glGetUniformLocation(g_modelProgram.program, "u_roughnessMaterial");
	g_R0MaterialModelLocation = glGetUniformLocation(g_modelProgram.program, "u_R0Material");

	g_numberSamplesModelLocation = glGetUniformLocation(g_modelProgram.program, "u_numberSamples");
	g_mModelLocation = glGetUniformLocation(g_modelProgram.program, "u_m");
	g_binaryFractionFactorModelLocation = glGetUniformLocation(g_modelProgram.program, "u_binaryFractionFactor");

	g_vertexModelLocation = glGetAttribLocation(g_modelProgram.program, "a_vertex");
	g_normalModelLocation = glGetAttribLocation(g_modelProgram.program, "a_normal");

	//

	glusFileLoadText("../Example32/shader/fullscreen.vert.glsl", &vertexSource);
	glusFileLoadText("../Example32/shader/fullscreen.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_fullscreenProgram, (const GLchar**)&vertexSource.text, 0, 0, 0, (const GLchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//

	g_framebufferTextureFullscreenLocation = glGetUniformLocation(g_fullscreenProgram.program, "u_framebufferTexture");

	g_msaaSamplesFullscreenLocation = glGetUniformLocation(g_fullscreenProgram.program, "u_msaaSamples");
	g_exposureFullscreenLocation = glGetUniformLocation(g_fullscreenProgram.program, "u_exposure");
	g_gammaFullscreenLocation = glGetUniformLocation(g_fullscreenProgram.program, "u_gamma");

	//
	//

	glusFileLoadText("../Example32/shader/background.vert.glsl", &vertexSource);
	glusFileLoadText("../Example32/shader/background.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_backgroundProgram, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//

	g_viewProjectionMatrixBackgroundLocation = glGetUniformLocation(g_backgroundProgram.program, "u_viewProjectionMatrix");
	g_panoramaTextureBackgroundLocation = glGetUniformLocation(g_backgroundProgram.program, "u_panoramaTexture");

	g_vertexBackgroundLocation = glGetAttribLocation(g_backgroundProgram.program, "a_vertex");

	//
	// Setting up the full screen frame buffer.
	//

	glGenTextures(1, &g_fullscreenTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, g_fullscreenTexture);

	// Create MSAA texture.
	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLES, GL_RGB32F, SCREEN_WIDTH, SCREEN_HEIGHT, GL_TRUE);

	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);

	// No need to access the depth buffer, so a render buffer is sufficient.

	glGenRenderbuffers(1, &g_fullscreenDepthRenderbuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, g_fullscreenDepthRenderbuffer);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLES, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//

	glGenFramebuffers(1, &g_fullscreenFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, g_fullscreenFBO);

	// Attach the color buffer ...
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, g_fullscreenTexture, 0);

	// ... and the depth buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_fullscreenDepthRenderbuffer);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

		return GLUS_FALSE;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//
	//
	//

	glGenTextures(1, &g_panoramaTexture);
	glBindTexture(GL_TEXTURE_2D, g_panoramaTexture);

	printf("Loading HDR texture ... ");
	if (!glusImageLoadHdr("doge2.hdr", &image))
	{
		printf("failed!\n");

		return GLUS_FALSE;
	}
	printf("done.\n");

	glGetError();
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, image.width, image.height, 0, GL_RGB, GL_FLOAT, image.data);
	glusImageDestroyHdr(&image);
	if (glGetError() != GL_NO_ERROR)
	{
		printf("Error creating float texture!\n");

		return GLUS_FALSE;
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glusShapeCreateSpheref(&backgroundSphere, 500.0f, 32);
	g_numberIndicesBackground = backgroundSphere.numberIndices;

	glGenBuffers(1, &g_verticesBackgroundVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
	glBufferData(GL_ARRAY_BUFFER, backgroundSphere.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)backgroundSphere.vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &g_indicesBackgroundVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, backgroundSphere.numberIndices * sizeof(GLuint), (GLuint*)backgroundSphere.indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glusShapeDestroyf(&backgroundSphere);

	//
	//

	// Use a helper function to load an wavefront object file.
	glusShapeLoadWavefront("venusm.obj", &wavefront);

	g_numberVerticesModel = wavefront.numberVertices;

	glGenBuffers(1, &g_verticesModelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_verticesModelVBO);
	glBufferData(GL_ARRAY_BUFFER, wavefront.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)wavefront.vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &g_normalsModelVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_normalsModelVBO);
	glBufferData(GL_ARRAY_BUFFER, wavefront.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)wavefront.normals, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glusShapeDestroyf(&wavefront);

	//

	glUseProgram(g_modelProgram.program);

	glUniform4fv(g_eyeModelLocation, 1, g_eye);
	glUniform1i(g_panoramaTextureModelLocation, 0);
	// Color is fixed.
	glUniform3fv(g_colorMaterialModelLocation, 1, colorMaterial);

	glGenVertexArrays(1, &g_modelVAO);
	glBindVertexArray(g_modelVAO);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesModelVBO);
	glVertexAttribPointer(g_vertexModelLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_vertexModelLocation);

	glBindBuffer(GL_ARRAY_BUFFER, g_normalsModelVBO);
	glVertexAttribPointer(g_normalModelLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_normalModelLocation);

	//

	glUseProgram(g_fullscreenProgram.program);

	glUniform1i(g_framebufferTextureFullscreenLocation, 0);
	glUniform1i(g_msaaSamplesFullscreenLocation, MSAA_SAMPLES);

	glGenVertexArrays(1, &g_fullscreenVAO);
	glBindVertexArray(g_fullscreenVAO);

	//

	glUseProgram(g_backgroundProgram.program);

	glUniform1i(g_panoramaTextureBackgroundLocation, 0);

	glGenVertexArrays(1, &g_backgroundVAO);
	glBindVertexArray(g_backgroundVAO);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesBackgroundVBO);
	glVertexAttribPointer(g_vertexBackgroundLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_vertexBackgroundLocation);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesBackgroundVBO);

	//

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f);

	glEnable(GL_CULL_FACE);

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	GLfloat projectionMatrix[16];
	GLfloat viewMatrix[16];

	glViewport(0, 0, width, height);

	glusMatrix4x4Perspectivef(projectionMatrix, 60.0f, (GLfloat)width / (GLfloat)height, 1.0f, 1000.0f);

	glusMatrix4x4LookAtf(viewMatrix, g_eye[0], g_eye[1], g_eye[2], 0.0f, 2.0f, 0.0f, 0.0f, 1.0f, 0.0f);

	glusMatrix4x4Multiplyf(g_viewProjectionMatrix, projectionMatrix, viewMatrix);
}

GLUSboolean update(GLUSfloat time)
{
	GLfloat modelMatrix[16];
	GLfloat normalMatrix[9];

	//
	// Render to FBO.
	//

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_panoramaTexture);

	glBindFramebuffer(GL_FRAMEBUFFER, g_fullscreenFBO);

	glEnable(GL_MULTISAMPLE);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Render the background.

	// Rendering the sphere from inside, so change front facing.
	glFrontFace(GL_CW);

	glUseProgram(g_backgroundProgram.program);

	glUniformMatrix4fv(g_viewProjectionMatrixBackgroundLocation, 1, GL_FALSE, g_viewProjectionMatrix);

	glBindVertexArray(g_backgroundVAO);

	glDrawElements(GL_TRIANGLES, g_numberIndicesBackground, GL_UNSIGNED_INT, 0);

	glFrontFace(GL_CCW);

	// Render model using BRDF and IBL.

	glusMatrix4x4Identityf(modelMatrix);
	glusMatrix4x4Scalef(modelMatrix, 0.001f, 0.001f, 0.001f);

	glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelMatrix);

	glUseProgram(g_modelProgram.program);

	glBindVertexArray(g_modelVAO);

	glUniform1ui(g_numberSamplesModelLocation, 1 << g_m);
	glUniform1ui(g_mModelLocation, g_m);
	// Results are in range [0.0 1.0] and not [0.0, 1.0[.
	glUniform1f(g_binaryFractionFactorModelLocation, 1.0f / (powf(2.0f, (GLfloat)g_m) - 1.0f));
	// Roughness of material.
	glUniform1f(g_roughnessMaterialModelLocation, g_roughness);
	glUniform1f(g_R0MaterialModelLocation, g_R0);

	glUniformMatrix4fv(g_viewProjectionMatrixModelLocation, 1, GL_FALSE, g_viewProjectionMatrix);
	glUniformMatrix4fv(g_modelMatrixModelLocation, 1, GL_FALSE, modelMatrix);
	glUniformMatrix3fv(g_normalMatrixModelLocation, 1, GL_FALSE, normalMatrix);

	glDrawArrays(GL_TRIANGLES, 0, g_numberVerticesModel);

	//
	// Render full screen to resolve the buffer: MSAA, tone mapping and gamma correction.
	//

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, g_fullscreenTexture);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDisable(GL_MULTISAMPLE);

	// No clear needed, as we just draw over the last content.
	glDisable(GL_DEPTH_TEST);

	glUseProgram(g_fullscreenProgram.program);

	glUniform1f(g_exposureFullscreenLocation, g_exposure);
	glUniform1f(g_gammaFullscreenLocation, g_gamma);

	glBindVertexArray(g_fullscreenVAO);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glEnable(GL_DEPTH_TEST);

	return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_panoramaTexture)
	{
		glDeleteTextures(1, &g_panoramaTexture);

		g_panoramaTexture = 0;
	}

	//

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (g_verticesModelVBO)
	{
		glDeleteBuffers(1, &g_verticesModelVBO);

		g_verticesModelVBO = 0;
	}

	if (g_normalsModelVBO)
	{
		glDeleteBuffers(1, &g_normalsModelVBO);

		g_normalsModelVBO = 0;
	}

	if (g_verticesBackgroundVBO)
	{
		glDeleteBuffers(1, &g_verticesBackgroundVBO);

		g_verticesBackgroundVBO = 0;
	}

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	if (g_indicesBackgroundVBO)
	{
		glDeleteBuffers(1, &g_indicesBackgroundVBO);

		g_indicesBackgroundVBO = 0;
	}

	glBindVertexArray(0);

	if (g_modelVAO)
	{
		glDeleteVertexArrays(1, &g_modelVAO);

		g_modelVAO = 0;
	}

	if (g_backgroundVAO)
	{
		glDeleteVertexArrays(1, &g_backgroundVAO);

		g_backgroundVAO = 0;
	}

	if (g_fullscreenVAO)
	{
		glDeleteVertexArrays(1, &g_fullscreenVAO);

		g_fullscreenVAO = 0;
	}

	glUseProgram(0);

	glusProgramDestroy(&g_modelProgram);

	glusProgramDestroy(&g_backgroundProgram);

	glusProgramDestroy(&g_fullscreenProgram);

	//

	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_fullscreenTexture)
	{
		glDeleteTextures(1, &g_fullscreenTexture);

		g_fullscreenTexture = 0;
	}

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	if (g_fullscreenDepthRenderbuffer)
	{
		glDeleteRenderbuffers(1, &g_fullscreenDepthRenderbuffer);

		g_fullscreenDepthRenderbuffer = 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (g_fullscreenFBO)
	{
		glDeleteFramebuffers(1, &g_fullscreenFBO);

		g_fullscreenFBO = 0;
	}
}

GLUSvoid key(const GLUSboolean pressed, const GLUSint key)
{
	if (pressed)
	{
		if (key == '1' && g_m > 1)
		{
			g_m--;
		}
		else if (key == '2' && g_m < MAX_M)
		{
			g_m++;
		}
		else if (key == '3')
		{
			g_roughness -= 0.1f;
		}
		else if (key == '4')
		{
			g_roughness += 0.1f;
		}
		else if (key == '5')
		{
			g_R0 -= 0.1f;
		}
		else if (key == '6')
		{
			g_R0 += 0.1f;
		}
	}

	g_roughness = glusMathClampf(g_roughness, 0.0f, 1.0f);
	g_R0 = glusMathClampf(g_R0, 0.0f, 1.0f);
}

int main(int argc, char* argv[])
{
	// No MSAA here, as we render to an off screen MSAA buffer.
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
    		EGL_CONTEXT_MAJOR_VERSION, 4,
    		EGL_CONTEXT_MINOR_VERSION, 1,
    		EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
    		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
    		EGL_NONE
    };

    glusWindowSetInitFunc(init);

    glusWindowSetReshapeFunc(reshape);

    glusWindowSetKeyFunc(key);

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

	// No resize, as it makes code simpler.
    if (!glusWindowCreate("GLUS Example Window", SCREEN_WIDTH, SCREEN_HEIGHT, GLUS_FALSE, GLUS_TRUE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    // Print out the keys
    printf("Keys:\n");
    printf("1       = Decrease number samples\n");
    printf("2       = Increase number samples\n");
    printf("3       = Decrease roughness\n");
    printf("4       = Increase roughness\n");
    printf("5       = Decrease R0\n");
    printf("6       = Increase R0\n");
    printf("\n");

    glusWindowRun();

    return 0;
}
