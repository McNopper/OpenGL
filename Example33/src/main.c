/**
 * OpenGL 4 - Example 33
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

// Number of roughness layers per specular cube map side
#define NUMBER_ROUGHNESS	6

#define SCREEN_WIDTH	1024
#define SCREEN_HEIGHT	768
#define MSAA_SAMPLES	4

static GLfloat g_exposure = 3.0f;
static GLfloat g_gamma = 2.2f;

// Roughness of the material.
static GLfloat g_roughness = 0.1f;

// Reflection coefficient see http://en.wikipedia.org/wiki/Schlick%27s_approximation
static GLfloat g_R0 = 0.2f;

// Base color.
static GLfloat g_colorMaterial[3] = { 0.8, 0.8, 0.8 };

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

static GLuint g_texture[3];

//
//
//

static GLUSprogram g_modelProgram;

static GLint g_viewProjectionMatrixModelLocation;

static GLint g_modelMatrixModelLocation;

static GLint g_normalMatrixModelLocation;

static GLint g_eyeModelLocation;

static GLint g_textureSpecularModelLocation;
static GLint g_textureDiffuseModelLocation;
static GLint g_textureLUTModelLocation;

static GLint g_colorMaterialModelLocation;
static GLint g_roughnessMaterialModelLocation;
static GLint g_roughnessScaleModelLocation;
static GLint g_R0MaterialModelLocation;

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

static GLint g_textureBackgroundLocation;

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

	// 6 sides of diffuse and specular; all roughness levels of specular.
	GLUShdrimage image[6 * NUMBER_ROUGHNESS + 6];

	// The look up table (LUT) is stored in a raw binary file.
	GLUSbinaryfile rawimage;

	GLUStextfile vertexSource;
	GLUStextfile fragmentSource;

	GLchar buffer[27] = "doge2/doge2_POS_X_00_s.hdr";

	GLint i, k, m;

	//

	glusFileLoadText("../Example33/shader/brdf.vert.glsl", &vertexSource);
	glusFileLoadText("../Example33/shader/brdf.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_modelProgram, (const GLchar**)&vertexSource.text, 0, 0, 0, (const GLchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	g_viewProjectionMatrixModelLocation = glGetUniformLocation(g_modelProgram.program, "u_viewProjectionMatrix");
	g_modelMatrixModelLocation = glGetUniformLocation(g_modelProgram.program, "u_modelMatrix");
	g_normalMatrixModelLocation = glGetUniformLocation(g_modelProgram.program, "u_normalMatrix");
	g_eyeModelLocation = glGetUniformLocation(g_modelProgram.program, "u_eye");
	g_textureSpecularModelLocation = glGetUniformLocation(g_modelProgram.program, "u_textureSpecular");
	g_textureDiffuseModelLocation = glGetUniformLocation(g_modelProgram.program, "u_textureDiffuse");
	g_textureLUTModelLocation = glGetUniformLocation(g_modelProgram.program, "u_textureLUT");
	g_colorMaterialModelLocation = glGetUniformLocation(g_modelProgram.program, "u_colorMaterial");
	g_roughnessMaterialModelLocation = glGetUniformLocation(g_modelProgram.program, "u_roughnessMaterial");
	g_roughnessScaleModelLocation = glGetUniformLocation(g_modelProgram.program, "u_roughnessScale");
	g_R0MaterialModelLocation = glGetUniformLocation(g_modelProgram.program, "u_R0Material");

	g_vertexModelLocation = glGetAttribLocation(g_modelProgram.program, "a_vertex");
	g_normalModelLocation = glGetAttribLocation(g_modelProgram.program, "a_normal");

	//

	glusFileLoadText("../Example33/shader/fullscreen.vert.glsl", &vertexSource);
	glusFileLoadText("../Example33/shader/fullscreen.frag.glsl", &fragmentSource);

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

	glusFileLoadText("../Example33/shader/background.vert.glsl", &vertexSource);
	glusFileLoadText("../Example33/shader/background.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_backgroundProgram, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//

	g_viewProjectionMatrixBackgroundLocation = glGetUniformLocation(g_backgroundProgram.program, "u_viewProjectionMatrix");
	g_textureBackgroundLocation = glGetUniformLocation(g_backgroundProgram.program, "u_texture");

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

	for (i = 0; i < 2; i++)
	{
		if (i == 0)
		{
			buffer[21] = 's';
		}
		else
		{
			buffer[21] = 'd';
		}

		for (k = 0; k < NUMBER_ROUGHNESS; k++)
		{
			if (i == 1 && k > 0)
			{
				continue;
			}

			buffer[18] = '0' + k / 10;
			buffer[19] = '0' + k % 10;

			for (m = 0; m < 6; m++)
			{
				if (m % 2 == 0)
				{
					buffer[12] = 'P';
					buffer[13] = 'O';
					buffer[14] = 'S';
				}
				else
				{
					buffer[12] = 'N';
					buffer[13] = 'E';
					buffer[14] = 'G';
				}

				switch (m)
				{
					case 0:
					case 1:
						buffer[16] = 'X';
						break;
					case 2:
					case 3:
						buffer[16] = 'Y';
						break;
					case 4:
					case 5:
						buffer[16] = 'Z';
						break;
				}

				printf("Loading '%s' ...", buffer);

				if (!glusImageLoadHdr(buffer, &image[i*NUMBER_ROUGHNESS*6 + k*6 + m]))
				{
					printf(" error!\n");
					continue;
				}

				printf(" done.\n");
			}
		}
	}

    glGenTextures(1, &g_texture[0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, g_texture[0]);

	glTexImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, GL_RGB32F, image[0].width, image[0].height, 6*NUMBER_ROUGHNESS, 0, GL_RGB, GL_FLOAT, 0);

	glusLogPrintError(GLUS_LOG_INFO, "glTexImage3D()");

    for (i = 0; i < NUMBER_ROUGHNESS; i++)
    {
        for (k = 0; k < 6; k++)
        {
        	glTexSubImage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 0, 0, 0,  6*i + k, image[i*6 + k].width, image[i*6 + k].height, 1, image[i*6 + k].format, GL_FLOAT, image[i*6 + k].data);

        	glusLogPrintError(GLUS_LOG_INFO, "glTexSubImage3D() %d %d", i, k);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

    //

    glGenTextures(1, &g_texture[1]);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, g_texture[1]);

    for (i = 0; i < 6; i++)
    {
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, image[i + 6*NUMBER_ROUGHNESS].format, image[i + 6*NUMBER_ROUGHNESS].width, image[i + 6*NUMBER_ROUGHNESS].height, 0, image[i + 6*NUMBER_ROUGHNESS].format, GL_FLOAT, image[i + 6*NUMBER_ROUGHNESS].data);

    	glusLogPrintError(GLUS_LOG_INFO, "glTexImage2D() %d", i);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    //

	printf("Loading 'doge2/EnvironmentBRDF_1024.data' ...");
    if (!glusFileLoadBinary("doge2/EnvironmentBRDF_1024.data", &rawimage))
    {
		printf(" error!\n");
    }
    else
    {
    	printf(" done.\n");
    }

    glGenTextures(1, &g_texture[2]);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, g_texture[2]);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, 1024, 1024, 0, GL_RG, GL_FLOAT, (GLfloat*)rawimage.binary);

    glusLogPrintError(GLUS_LOG_INFO, "glTexImage2D()");

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    glusFileDestroyBinary(&rawimage);

    //

	for (i = 0; i < 2; i++)
	{
		for (k = 0; k < NUMBER_ROUGHNESS; k++)
		{
			if (i == 1 && k > 0)
			{
				continue;
			}

			for (m = 0; m < 6; m++)
			{
				glusImageDestroyHdr(&image[i*NUMBER_ROUGHNESS*6 + k*6 + m]);
			}
		}
	}

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
	glUniform1i(g_textureSpecularModelLocation, 0);
	glUniform1i(g_textureDiffuseModelLocation, 1);
	glUniform1i(g_textureLUTModelLocation, 2);
	glUniform1f(g_roughnessScaleModelLocation, (GLfloat)(NUMBER_ROUGHNESS - 1));

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

	glUniform1i(g_textureBackgroundLocation, 0);

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
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, g_texture[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_CUBE_MAP, g_texture[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_texture[2]);

	glActiveTexture(GL_TEXTURE0);

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

	// Roughness of material.
	glUniform1f(g_roughnessMaterialModelLocation, g_roughness);
	glUniform1f(g_R0MaterialModelLocation, g_R0);
	glUniform3fv(g_colorMaterialModelLocation, 1, g_colorMaterial);

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
	glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

	if (g_texture[0])
	{
		glDeleteTextures(1, &g_texture[0]);

		g_texture[0] = 0;
	}

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	if (g_texture[1])
	{
		glDeleteTextures(1, &g_texture[1]);

		g_texture[1] = 0;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_texture[2])
	{
		glDeleteTextures(1, &g_texture[2]);

		g_texture[2] = 0;
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
		if (key == '1')
		{
			g_roughness -= 0.1f;
		}
		else if (key == '2')
		{
			g_roughness += 0.1f;
		}
		else if (key == '3')
		{
			g_R0 -= 0.1f;
		}
		else if (key == '4')
		{
			g_R0 += 0.1f;
		}
		else if (key == '5')
		{
			g_colorMaterial[0] -= 0.1f;
		}
		else if (key == '6')
		{
			g_colorMaterial[0] += 0.1f;
		}
		else if (key == '7')
		{
			g_colorMaterial[1] -= 0.1f;
		}
		else if (key == '8')
		{
			g_colorMaterial[1] += 0.1f;
		}
		else if (key == '9')
		{
			g_colorMaterial[2] -= 0.1f;
		}
		else if (key == '0')
		{
			g_colorMaterial[2] += 0.1f;
		}
	}

	g_roughness = glusMathClampf(g_roughness, 0.0f, 1.0f);
	g_R0 = glusMathClampf(g_R0, 0.0f, 1.0f);

	g_colorMaterial[0] = glusMathClampf(g_colorMaterial[0], 0.0f, 1.0f);
	g_colorMaterial[1] = glusMathClampf(g_colorMaterial[1], 0.0f, 1.0f);
	g_colorMaterial[2] = glusMathClampf(g_colorMaterial[2], 0.0f, 1.0f);
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
    printf("1       = Decrease roughness\n");
    printf("2       = Increase roughness\n");
    printf("3       = Decrease R0\n");
    printf("4       = Increase R0\n");
    printf("5       = Decrease Red\n");
    printf("6       = Increase Red\n");
    printf("7       = Decrease Green\n");
    printf("8       = Increase Green\n");
    printf("9       = Decrease Blue\n");
    printf("0       = Increase Blue\n");
    printf("\n");

    glusWindowRun();

    return 0;
}
