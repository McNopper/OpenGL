/**
 * OpenGL 4 - Example 28
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * see http://john-chapman-graphics.blogspot.co.uk/2013/01/ssao-tutorial.html
 */

#include <stdio.h>

#include "GL/glus.h"

#include "wavefront.h"

#define KERNEL_SIZE 32

#define SSAO_RADIUS 1.0f

#define ROTATION_NOISE_SIDE_LENGTH 4
#define ROTATION_NOISE_SIZE (ROTATION_NOISE_SIDE_LENGTH*ROTATION_NOISE_SIDE_LENGTH)

//

static GLfloat g_viewMatrix[16];

static GLfloat g_viewProjectionMatrix[16];

static GLfloat g_inverseProjectionMatrix[16];

static GLfloat g_projectionMatrix[16];

//

static GLUSprogram g_program;

static GLint g_viewProjectionMatrixLocation;

static GLint g_modelMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_repeatLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLint g_texCoordLocation;

static GLint g_textureLocation;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_texCoordsVBO;

static GLuint g_indicesVBO;

static GLuint g_numberIndicesPlane;

//

static GLuint g_vao;

//

static GLuint g_texture;

//
//
//

static GLuint g_ssaoTexture;

static GLuint g_ssaoDepthTexture;

static GLuint g_ssaoNormalTexture;

static GLuint g_ssaoFBO;

static GLuint g_ssaoRotationNoiseTexture;

//

static GLuint g_blurTexture;

static GLuint g_blurFBO;

//
//
//

static GLUSprogram g_ssaoProgram;

static GLint g_ssaoVertexLocation;

static GLint g_ssaoTexCoordLocation;

static GLint g_ssaoTextureLocation;

static GLint g_ssaoNormalTextureLocation;

static GLint g_ssaoDepthTextureLocation;

static GLint g_ssaoKernelLocation;

static GLint g_ssaoRotationNoiseTextureLocation;

static GLint g_ssaoRotationNoiseScaleLocation;

static GLint g_ssaoInverseProjectionMatrixLocation;

static GLint g_ssaoProjectionMatrixLocation;

static GLint g_ssaoRadiusLocation;

//

static GLUSprogram g_blurProgram;

static GLint g_blurVertexLocation;

static GLint g_blurTexCoordLocation;

static GLint g_blurColorTextureLocation;

static GLint g_blurSSAOTextureLocation;

static GLint g_blurTexelStepLocation;

static GLint g_blurNoSSAOLocation;

//

static GLuint g_postprocessVerticesVBO;

static GLuint g_postprocessTexCoordsVBO;

static GLuint g_postprocessIndicesVBO;

static GLuint g_numberIndicesPostprocessPlane;

//

static GLuint g_ssaoVAO;

static GLuint g_blurVAO;

//

static struct LightProperties g_light = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

static struct CameraProperties g_camera = { { 0.0f, 15.0f, 15.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } };

static GLfloat g_kernel[3 * KERNEL_SIZE];

static GLfloat g_rotationNoise[3 * ROTATION_NOISE_SIZE];

static GLfloat g_rotationNoiseScale[2];

static GLfloat g_texelStep[2];

GLUSboolean init(GLUSvoid)
{
	GLUStextfile vertexSource;
	GLUStextfile fragmentSource;

	GLUStgaimage image;

	GLUSshape plane;

	GLint i;

	//

	glusMatrix4x4LookAtf(g_viewMatrix, g_camera.eye[0], g_camera.eye[1], g_camera.eye[2], g_camera.center[0], g_camera.center[1], g_camera.center[2], g_camera.up[0], g_camera.up[1], g_camera.up[2]);

	//

	if (!initWavefront(g_viewMatrix, &g_light))
	{
		return GLUS_FALSE;
	}

	//

	glusFileLoadText("../Example28/shader/texture.vert.glsl", &vertexSource);
	glusFileLoadText("../Example28/shader/texture.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_program, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//

	// Retrieve the uniform locations in the program.
	g_viewProjectionMatrixLocation = glGetUniformLocation(g_program.program, "u_viewProjectionMatrix");
	g_modelMatrixLocation = glGetUniformLocation(g_program.program, "u_modelMatrix");
	g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
	g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");
	g_repeatLocation = glGetUniformLocation(g_program.program, "u_repeat");
	g_textureLocation = glGetUniformLocation(g_program.program, "u_texture");

	// Retrieve the attribute locations in the program.
	g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
	g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");
	g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");

	//
	// SSAO shader etc.
	//

	glusFileLoadText("../Example28/shader/ssao.vert.glsl", &vertexSource);
	glusFileLoadText("../Example28/shader/ssao.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_ssaoProgram, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//

	// Retrieve the uniform locations in the program.
	g_ssaoTextureLocation = glGetUniformLocation(g_ssaoProgram.program, "u_texture");
	g_ssaoNormalTextureLocation = glGetUniformLocation(g_ssaoProgram.program, "u_normalTexture");
	g_ssaoDepthTextureLocation = glGetUniformLocation(g_ssaoProgram.program, "u_depthTexture");
	g_ssaoKernelLocation = glGetUniformLocation(g_ssaoProgram.program, "u_kernel");
	g_ssaoRotationNoiseTextureLocation = glGetUniformLocation(g_ssaoProgram.program, "u_rotationNoiseTexture");
	g_ssaoRotationNoiseScaleLocation = glGetUniformLocation(g_ssaoProgram.program, "u_rotationNoiseScale");
	g_ssaoInverseProjectionMatrixLocation = glGetUniformLocation(g_ssaoProgram.program, "u_inverseProjectionMatrix");
	g_ssaoProjectionMatrixLocation = glGetUniformLocation(g_ssaoProgram.program, "u_projectionMatrix");
	g_ssaoRadiusLocation = glGetUniformLocation(g_ssaoProgram.program, "u_radius");

	// Retrieve the attribute locations in the program.
	g_ssaoVertexLocation = glGetAttribLocation(g_ssaoProgram.program, "a_vertex");
	g_ssaoTexCoordLocation = glGetAttribLocation(g_ssaoProgram.program, "a_texCoord");

	//
	// Blur shader etc.
	//

	glusFileLoadText("../Example28/shader/blur.vert.glsl", &vertexSource);
	glusFileLoadText("../Example28/shader/blur.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_blurProgram, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//

	// Retrieve the uniform locations in the program.
	g_blurColorTextureLocation = glGetUniformLocation(g_blurProgram.program, "u_colorTexture");

	g_blurSSAOTextureLocation = glGetUniformLocation(g_blurProgram.program, "u_ssaoTexture");

	g_blurTexelStepLocation = glGetUniformLocation(g_blurProgram.program, "u_texelStep");

	g_blurNoSSAOLocation = glGetUniformLocation(g_blurProgram.program, "u_noSSAO");

	// Retrieve the attribute locations in the program.
	g_blurVertexLocation = glGetAttribLocation(g_blurProgram.program, "a_vertex");
	g_blurTexCoordLocation = glGetAttribLocation(g_blurProgram.program, "a_texCoord");

	//
	// Texture set up for the ground plane.
	//

	glusImageLoadTga("wood_texture.tga", &image);

	glGenTextures(1, &g_texture);
	glBindTexture(GL_TEXTURE_2D, g_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

	// Mipmap generation is now included in OpenGL 3 and above
	glGenerateMipmap(GL_TEXTURE_2D);

	// Trilinear filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	//
	// Setting up the SSAO frame buffer.
	//

	glGenTextures(1, &g_ssaoTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_ssaoTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GLUS_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GLUS_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glGenTextures(1, &g_ssaoNormalTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_ssaoNormalTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GLUS_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GLUS_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glGenTextures(1, &g_ssaoDepthTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_ssaoDepthTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glGenFramebuffers(1, &g_ssaoFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, g_ssaoFBO);

	// Attach the color buffer ...
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_ssaoTexture, 0);

	// Attach the normal buffer ...
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, g_ssaoNormalTexture, 0);

	// ... and the depth buffer,
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_ssaoDepthTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

		return GLUS_FALSE;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//
	// Setting up the blur frame buffer
	//

	glGenTextures(1, &g_blurTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_blurTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GLUS_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GLUS_RGB, GL_UNSIGNED_BYTE, 0);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glGenFramebuffers(1, &g_blurFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, g_blurFBO);

	// Attach the color buffer ...
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_blurTexture, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

		return GLUS_FALSE;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//
	// Ground plane setup.
	//

	glusShapeCreatePlanef(&plane, 20.0f);

	g_numberIndicesPlane = plane.numberIndices;

	glGenBuffers(1, &g_verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)plane.vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &g_normalsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
	glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 3 * sizeof(GLfloat), (GLfloat*)plane.normals, GL_STATIC_DRAW);

	glGenBuffers(1, &g_texCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 2 * sizeof(GLfloat), (GLfloat*)plane.texCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &g_indicesVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, plane.numberIndices * sizeof(GLuint), (GLuint*)plane.indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glusShapeDestroyf(&plane);

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

	glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
	glVertexAttribPointer(g_texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_texCoordLocation);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

	glBindVertexArray(0);

	//

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture);
	glUniform1i(g_textureLocation, 0);

	// How many times the surface texture is repeated.
	glUniform1f(g_repeatLocation, 6.0f);

	//
	// Post process plane setup.
	//

	glusShapeCreatePlanef(&plane, 1.0f);

	g_numberIndicesPostprocessPlane = plane.numberIndices;

	glGenBuffers(1, &g_postprocessVerticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_postprocessVerticesVBO);
	glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)plane.vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &g_postprocessTexCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_postprocessTexCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 2 * sizeof(GLfloat), (GLfloat*)plane.texCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &g_postprocessIndicesVBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_postprocessIndicesVBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, plane.numberIndices * sizeof(GLuint), (GLuint*)plane.indices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	glusShapeDestroyf(&plane);

	//

	glUseProgram(g_ssaoProgram.program);

	glGenVertexArrays(1, &g_ssaoVAO);
	glBindVertexArray(g_ssaoVAO);

	glBindBuffer(GL_ARRAY_BUFFER, g_postprocessVerticesVBO);
	glVertexAttribPointer(g_ssaoVertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_ssaoVertexLocation);

	glBindBuffer(GL_ARRAY_BUFFER, g_postprocessTexCoordsVBO);
	glVertexAttribPointer(g_ssaoTexCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_ssaoTexCoordLocation);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_postprocessIndicesVBO);

	glBindVertexArray(0);

	//

	glUniform1i(g_ssaoTextureLocation, 0);
	glUniform1i(g_ssaoNormalTextureLocation, 1);
	glUniform1i(g_ssaoDepthTextureLocation, 2);
	glUniform1i(g_ssaoRotationNoiseTextureLocation, 3);

	glUniform1f(g_ssaoRadiusLocation, SSAO_RADIUS);

	//
	// Create the Kernel for SSAO.
	//

	for (i = 0; i < KERNEL_SIZE; i++)
	{
		g_kernel[i * 3 + 0] = glusRandomUniformf(-1.0f, 1.0f);
		g_kernel[i * 3 + 1] = glusRandomUniformf(-1.0f, 1.0f);
		g_kernel[i * 3 + 2] = glusRandomUniformf(0.0f, 1.0f);	// Kernel hemisphere points to positive Z-Axis.

		glusVector3Normalizef(&g_kernel[i * 3]);					// Normalize, so included in the hemisphere.

		GLfloat scale = (GLfloat)i / (GLfloat)KERNEL_SIZE;		// Create a scale value between [0;1[ .

		scale = glusMathClampf(scale * scale, 0.1f, 1.0f);			// Adjust scale, that there are more values closer to the center of the g_kernel.

		glusVector3MultiplyScalarf(&g_kernel[i * 3], &g_kernel[i * 3], scale);
	}

	// Pass g_kernel to shader
	glUniform3fv(g_ssaoKernelLocation, KERNEL_SIZE, g_kernel);

	//
	// Create the rotation noise texture
	//

	for (i = 0; i < ROTATION_NOISE_SIZE; i++)
	{
		g_rotationNoise[i * 3 + 0] = glusRandomUniformf(-1.0f, 1.0f);
		g_rotationNoise[i * 3 + 1] = glusRandomUniformf(-1.0f, 1.0f);
		g_rotationNoise[i * 3 + 2] = 0.0f;						// Rotate on x-y-plane, so z is zero.

		glusVector3Normalizef(&g_rotationNoise[i * 3]);			// Normalized rotation vector.
	}

	//

	glGenTextures(1, &g_ssaoRotationNoiseTexture);
	glBindTexture(GL_TEXTURE_2D, g_ssaoRotationNoiseTexture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, ROTATION_NOISE_SIDE_LENGTH, ROTATION_NOISE_SIDE_LENGTH, 0, GL_RGB, GL_FLOAT, g_rotationNoise);

	// No filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	//
	//

	g_rotationNoiseScale[0] = (GLfloat)TEXTURE_WIDTH / (GLfloat)ROTATION_NOISE_SIDE_LENGTH;
	g_rotationNoiseScale[1] = (GLfloat)TEXTURE_HEIGHT / (GLfloat)ROTATION_NOISE_SIDE_LENGTH;

	// Pass the scale, as the rotation noise texture is repeated over the screen x / y times.
	glUniform2fv(g_ssaoRotationNoiseScaleLocation, 1, g_rotationNoiseScale);

	//
	//

	glUseProgram(g_blurProgram.program);

	glGenVertexArrays(1, &g_blurVAO);
	glBindVertexArray(g_blurVAO);

	glBindBuffer(GL_ARRAY_BUFFER, g_postprocessVerticesVBO);
	glVertexAttribPointer(g_blurVertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_blurVertexLocation);

	glBindBuffer(GL_ARRAY_BUFFER, g_postprocessTexCoordsVBO);
	glVertexAttribPointer(g_blurTexCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_blurTexCoordLocation);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_postprocessIndicesVBO);

	glBindVertexArray(0);

	//

	glUniform1i(g_blurColorTextureLocation, 0);

	glUniform1i(g_blurSSAOTextureLocation, 1);

	g_texelStep[0] = 1.0f / (GLfloat)TEXTURE_WIDTH;
	g_texelStep[1] = 1.0f / (GLfloat)TEXTURE_HEIGHT;

	// Pass the value to step from one to another texel.
	glUniform2fv(g_blurTexelStepLocation, 1, g_texelStep);

	// Variable to toggle between SSAO on and off
	glUniform1f(g_blurNoSSAOLocation, 0.0f);

	//
	// Basic OpenGL set up.
	//

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClearDepth(1.0f);

	glEnable(GL_DEPTH_TEST);

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	GLfloat modelMatrix[16];
	GLfloat modelViewMatrix[16];
	GLfloat normalMatrix[9];
	GLfloat lightDirection[3];

	reshapeWavefront(width, height);

	//

	glViewport(0, 0, width, height);

	glusMatrix4x4Perspectivef(g_projectionMatrix, 40.0f, (GLfloat)width / (GLfloat)height, 1.0f, 100.0f);

	// Calculate the inverse. Needed for the SSAO shader to get from projection to view space.

	glusMatrix4x4Copyf(g_inverseProjectionMatrix, g_projectionMatrix, GLUS_FALSE);

	glusMatrix4x4Inversef(g_inverseProjectionMatrix);

	glUseProgram(g_ssaoProgram.program);

	glUniformMatrix4fv(g_ssaoInverseProjectionMatrixLocation, 1, GL_FALSE, g_inverseProjectionMatrix);

	glUniformMatrix4fv(g_ssaoProjectionMatrixLocation, 1, GL_FALSE, g_projectionMatrix);

	//

	glusMatrix4x4Multiplyf(g_viewProjectionMatrix, g_projectionMatrix, g_viewMatrix);

	glUseProgram(g_program.program);

	glUniformMatrix4fv(g_viewProjectionMatrixLocation, 1, GL_FALSE, g_viewProjectionMatrix);

	glusMatrix4x4Identityf(modelMatrix);
	glusMatrix4x4RotateRxf(modelMatrix, -90.0f);
	glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);

	// Calculation is in camera space
	glusMatrix4x4Multiplyf(modelViewMatrix, g_viewMatrix, modelMatrix);
	glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);
	glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

	glusMatrix4x4MultiplyVector3f(lightDirection, g_viewMatrix, g_light.direction);
	// Direction already normalized
	glUniform3fv(g_lightDirectionLocation, 1, lightDirection);
}

GLUSvoid key(GLUSboolean pressed, GLUSint key)
{
	static GLboolean noSSAO = GL_FALSE;

	if (pressed)
	{
		// w for wireframe on / off
		if (key == 's')
		{
			noSSAO = !noSSAO;

			if (noSSAO)
			{
				glUniform1f(g_blurNoSSAOLocation, 1.0f);
			}
			else
			{
				glUniform1f(g_blurNoSSAOLocation, 0.0f);
			}
		}
	}
}

GLUSboolean update(GLUSfloat time)
{
	GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };

	GLfloat modelMatrix[16];

	GLint i;

	//
	// Rendering scene to frame buffer.
	//

	glBindFramebuffer(GL_FRAMEBUFFER, g_ssaoFBO);
	glDrawBuffers(2, drawBuffers);

	reshape(TEXTURE_WIDTH, TEXTURE_HEIGHT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (i = 0; i < 3; i++)
	{
		glusMatrix4x4Identityf(modelMatrix);
		glusMatrix4x4Translatef(modelMatrix, (GLfloat)(i - 1) * 4.1f, 0.0f, 0.0f);
		glusMatrix4x4RotateRyf(modelMatrix, 45.0f);

		if (!updateWavefront(time, modelMatrix))
		{
			return GLUS_FALSE;
		}
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_texture);

	glUseProgram(g_program.program);

	glBindVertexArray(g_vao);

	glDrawElements(GL_TRIANGLES, g_numberIndicesPlane, GL_UNSIGNED_INT, 0);

	//
	// SSAO rendering step.
	//

	glBindFramebuffer(GL_FRAMEBUFFER, g_blurFBO);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glClear(GL_COLOR_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_ssaoTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_ssaoNormalTexture);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, g_ssaoDepthTexture);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, g_ssaoRotationNoiseTexture);
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(g_ssaoProgram.program);

	glBindVertexArray(g_ssaoVAO);

	glDrawElements(GL_TRIANGLES, g_numberIndicesPostprocessPlane, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

	//
	// Blur rendering step.
	//

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, g_ssaoTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_blurTexture);
	glActiveTexture(GL_TEXTURE0);

	glUseProgram(g_blurProgram.program);

	glBindVertexArray(g_blurVAO);

	glDrawElements(GL_TRIANGLES, g_numberIndicesPostprocessPlane, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);

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

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_texture)
	{
		glDeleteTextures(1, &g_texture);

		g_texture = 0;
	}

	glBindVertexArray(0);

	if (g_vao)
	{
		glDeleteVertexArrays(1, &g_vao);

		g_vao = 0;
	}

	glUseProgram(0);

	glusProgramDestroy(&g_program);

	//

	if (g_postprocessVerticesVBO)
	{
		glDeleteBuffers(1, &g_postprocessVerticesVBO);

		g_postprocessVerticesVBO = 0;
	}

	if (g_postprocessTexCoordsVBO)
	{
		glDeleteBuffers(1, &g_postprocessTexCoordsVBO);

		g_postprocessTexCoordsVBO = 0;
	}

	if (g_postprocessIndicesVBO)
	{
		glDeleteBuffers(1, &g_postprocessIndicesVBO);

		g_postprocessIndicesVBO = 0;
	}

	if (g_ssaoVAO)
	{
		glDeleteVertexArrays(1, &g_ssaoVAO);

		g_ssaoVAO = 0;
	}

	if (g_blurVAO)
	{
		glDeleteVertexArrays(1, &g_blurVAO);

		g_blurVAO = 0;
	}

	glusProgramDestroy(&g_ssaoProgram);

	glusProgramDestroy(&g_blurProgram);

	//

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_ssaoTexture)
	{
		glDeleteTextures(1, &g_ssaoTexture);

		g_ssaoTexture = 0;
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_ssaoNormalTexture)
	{
		glDeleteTextures(1, &g_ssaoNormalTexture);

		g_ssaoNormalTexture = 0;
	}

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_ssaoDepthTexture)
	{
		glDeleteTextures(1, &g_ssaoDepthTexture);

		g_ssaoDepthTexture = 0;
	}

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_ssaoRotationNoiseTexture)
	{
		glDeleteTextures(1, &g_ssaoRotationNoiseTexture);

		g_ssaoRotationNoiseTexture = 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (g_ssaoFBO)
	{
		glDeleteFramebuffers(1, &g_ssaoFBO);

		g_ssaoFBO = 0;
	}

	//

	if (g_blurTexture)
	{
		glDeleteTextures(1, &g_blurTexture);

		g_blurTexture = 0;
	}

	if (g_blurFBO)
	{
		glDeleteFramebuffers(1, &g_blurFBO);

		g_blurFBO = 0;
	}

	//

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

	// No resizing for convenience. If resizing is allowed, dynamically resize the SSAO and Blur frame buffer as well.
    if (!glusWindowCreate("GLUS Example Window", TEXTURE_WIDTH, TEXTURE_HEIGHT, GLUS_FALSE, GLUS_TRUE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    // Print out the keys
    printf("Keys:\n");
    printf("s = Toggle SSAO on/off\n");

    glusWindowRun();

    return 0;
}
