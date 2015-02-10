/**
 * OpenGL 3 - Example 19
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

static GLint g_viewProjectionMatrixLocation;

static GLint g_viewProjectionBiasTextureMatrixLocation;

static GLint g_modelMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_repeatLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLint g_texCoordLocation;

static GLint g_textureLocation;

static GLint g_mirrorTextureLocation;

//

GLfloat g_viewProjectionMatrix[16];

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_texCoordsVBO;

static GLuint g_indicesVBO;

//

static GLuint g_vao;

//

static GLuint g_texture;

//

static GLuint g_numberIndicesSphere;

//

static GLuint g_mirrorTexture;

static GLuint g_depthMirrorTexture;

static GLuint g_fboMirrorTexture;

//

//
// Needed store the current value.
//

static GLuint g_width;

static GLuint g_height;

//

static struct LightProperties g_light = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

static struct CameraProperties g_camera = { {0.0f, 1.0f, 7.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};

GLUSboolean init(GLUSvoid)
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUStgaimage image;

    GLUSshape plane;

    //

    glusMatrix4x4LookAtf(g_viewMatrix, g_camera.eye[0], g_camera.eye[1], g_camera.eye[2], g_camera.center[0], g_camera.center[1], g_camera.center[2], g_camera.up[0], g_camera.up[1], g_camera.up[2]);

    //

    if (!initWavefront(g_viewMatrix, &g_light))
    {
    	return GLUS_FALSE;
    }

    //

    glusFileLoadText("../Example19/shader/basic_proj.vert.glsl", &vertexSource);
    glusFileLoadText("../Example19/shader/texture_multi_proj.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    // Retrieve the uniform locations in the program.
    g_viewProjectionMatrixLocation = glGetUniformLocation(g_program.program, "u_viewProjectionMatrix");
    g_viewProjectionBiasTextureMatrixLocation = glGetUniformLocation(g_program.program, "u_viewProjectionBiasTextureMatrix");
    g_modelMatrixLocation = glGetUniformLocation(g_program.program, "u_modelMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");
    g_repeatLocation =  glGetUniformLocation(g_program.program, "u_repeat");
    g_textureLocation = glGetUniformLocation(g_program.program, "u_texture");
    g_mirrorTextureLocation = glGetUniformLocation(g_program.program, "u_mirrorTexture");

    // Retrieve the attribute locations in the program.
    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");
    g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");

    //

    // Texture set up.

    glusImageLoadTga("ice.tga", &image);

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
    // Setting up the offscreen frame buffer.
    //

    glGenTextures(1, &g_mirrorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_mirrorTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GLUS_RGB, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GLUS_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    //

    glGenRenderbuffers(1, &g_depthMirrorTexture);
    glBindRenderbuffer(GL_RENDERBUFFER, g_depthMirrorTexture);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, TEXTURE_WIDTH, TEXTURE_HEIGHT);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    //

    glGenFramebuffers(1, &g_fboMirrorTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, g_fboMirrorTexture);

    // Attach the color buffer ...
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_mirrorTexture, 0);

    // ... and the depth buffer,
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_depthMirrorTexture);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

        return GLUS_FALSE;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //
    //
    //

    glusShapeCreatePlanef(&plane, 3.0f);

    g_numberIndicesSphere = plane.numberIndices;

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) plane.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) plane.normals, GL_STATIC_DRAW);

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

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_normalLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
    glVertexAttribPointer(g_texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_texCoordLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

    //

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glUniform1i(g_textureLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_mirrorTexture);
    glUniform1i(g_mirrorTextureLocation, 1);

    // How many times the surface texture is repeated.
    glUniform1f(g_repeatLocation, 6.0f);

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
    GLfloat modelViewMatrix[16];
    GLfloat normalMatrix[9];
    GLfloat lightDirection[3];

    g_width = width;
    g_height = height;

    reshapeWavefront(width, height);

    //

    glViewport(0, 0, width, height);

    glusMatrix4x4Perspectivef(g_viewProjectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    glusMatrix4x4Multiplyf(g_viewProjectionMatrix, g_viewProjectionMatrix, g_viewMatrix);

    glUseProgram(g_program.program);

    glUniformMatrix4fv(g_viewProjectionMatrixLocation, 1, GL_FALSE, g_viewProjectionMatrix);

    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4Translatef(modelMatrix, 0.0f, 0.0f, 3.0f);
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

GLUSboolean update(GLUSfloat time)
{
	// Bias needed to convert the from [-1;1] to [0;1]
	GLfloat biasMatrix[16] = {0.5f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.5f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.5f, 0.0f,
								0.5f, 0.5f, 0.5f, 1.0f};
	// Frame buffer has another view port and so perspective projection. Needed for projected texturing of the mirror texture.
	GLfloat viewProjectionBiasTextureMatrix[16];

	// This matrix is used to flip the rendered object upside down.
	GLfloat scaleMatrix[16];

	// Store current width and height for later reseting.
	GLuint width = g_width;
	GLuint height = g_height;

	//
    // Upside down rendering to frame buffer.
	//
    glBindFramebuffer(GL_FRAMEBUFFER, g_fboMirrorTexture);

	reshape(TEXTURE_WIDTH, TEXTURE_HEIGHT);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glusMatrix4x4Identityf(scaleMatrix);
    glusMatrix4x4Scalef(scaleMatrix, 1.0f, -1.0f, 1.0f);

    glFrontFace(GL_CW);

    if (!updateWavefront(time, scaleMatrix))
    {
    	return GLUS_FALSE;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Save the current projection matrix for later usage.

    glusMatrix4x4Copyf(viewProjectionBiasTextureMatrix, g_viewProjectionMatrix, GLUS_FALSE);

    glusMatrix4x4Multiplyf(viewProjectionBiasTextureMatrix, biasMatrix, viewProjectionBiasTextureMatrix);

    //
    // Scene rendering
    //

    reshape(width, height);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Normal rendering
    glusMatrix4x4Identityf(scaleMatrix);

    glFrontFace(GL_CCW);

    if (!updateWavefront(time, scaleMatrix))
    {
    	return GLUS_FALSE;
    }

    glUseProgram(g_program.program);

    // This matrix is needed to calculate the vertices into the frame buffer render pass.
    glUniformMatrix4fv(g_viewProjectionBiasTextureMatrixLocation, 1, GL_FALSE, viewProjectionBiasTextureMatrix);

    glBindVertexArray(g_vao);

    glDrawElements(GL_TRIANGLES, g_numberIndicesSphere, GL_UNSIGNED_INT, 0);

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

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, 0);

    if (g_mirrorTexture)
    {
        glDeleteTextures(1, &g_mirrorTexture);

        g_mirrorTexture = 0;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if (g_depthMirrorTexture)
    {
        glDeleteRenderbuffers(1, &g_depthMirrorTexture);

        g_depthMirrorTexture = 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (g_fboMirrorTexture)
	{
		glDeleteFramebuffers(1, &g_fboMirrorTexture);

		g_fboMirrorTexture = 0;
	}

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

    if (!glusWindowCreate("GLUS Example Window", TEXTURE_WIDTH, TEXTURE_HEIGHT, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
