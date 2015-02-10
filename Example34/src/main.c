/**
 * OpenGL 3 - Example 34
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

/**
 * Width of the viewport, needed to reset from framebuffer.
 */
static GLUSuint g_width;

/**
 * Height of the viewport.
 */
static GLUSuint g_height;

static GLUSfloat g_cameraPosition[3] = { 0.0f, 0.0f, 3.0f };

static GLUSfloat g_lightPosition[3] = { 2.0f, 2.0f, 3.0f };

static GLUSfloat g_near = 1.0f;

static GLUSfloat g_far = 100.0f;

static GLUSfloat g_wrap = 0.1f;

static GLUSfloat g_scatterWidth = 0.5f;

static GLUSfloat g_scatterFalloff = 5.0f;

/**
 * Matrix, to convert from scene world space to depth pass projection space.
 */
static GLfloat g_depthPassMatrix[16];

static GLUSprogram g_program;

static GLint g_depthPassMatrixLocation;

static GLint g_projectionMatrixLocation;

static GLint g_viewMatrixLocation;

static GLint g_modelMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_depthPassTextureLocation;

static GLint g_lightDirectionLocation;

static GLint g_diffuseColorLocation;

static GLint g_scatterColorLocation;

static GLint g_nearFarLocation;

static GLint g_wrapLocation;

static GLint g_scatterWidthLocation;

static GLint g_scatterFalloffLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

//

static GLUSprogram g_programDepthPass;

static GLint g_projectionMatrixDepthPassLocation;

static GLint g_modelViewMatrixDepthPassLocation;

static GLint g_vertexDepthPassLocation;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

//

static GLuint g_vao;

static GLuint g_vaoDepthPass;

//

static GLuint g_numberVertices;

//

/**
 * The pepth pass texture size.
 */
static const GLuint g_depthPassTextureSize = 4096;

/**
 * The depth pass texture.
 */
static GLuint g_depthPassTexture;

/**
 * The frame buffer object for the depth pass texture.
 */
static GLuint g_fbo;

GLUSboolean init(GLUSvoid)
{
    GLUSshape wavefrontObj;

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLfloat viewMatrix[16];

    GLfloat lightDirection[3];

    lightDirection[0] = g_lightPosition[0];
    lightDirection[1] = g_lightPosition[1];
    lightDirection[2] = g_lightPosition[2];

    glusVector3Normalizef(lightDirection);

    //

    glusFileLoadText("../Example34/shader/renderdepthmap.vert.glsl", &vertexSource);
    glusFileLoadText("../Example34/shader/renderdepthmap.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_programDepthPass, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    glusFileLoadText("../Example34/shader/subsurfacescattering.vert.glsl", &vertexSource);
    glusFileLoadText("../Example34/shader/subsurfacescattering.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixDepthPassLocation = glGetUniformLocation(g_programDepthPass.program, "u_projectionMatrix");
    g_modelViewMatrixDepthPassLocation = glGetUniformLocation(g_programDepthPass.program, "u_modelViewMatrix");
    g_vertexDepthPassLocation = glGetAttribLocation(g_programDepthPass.program, "a_vertex");

    //

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_viewMatrixLocation = glGetUniformLocation(g_program.program, "u_viewMatrix");
    g_modelMatrixLocation = glGetUniformLocation(g_program.program, "u_modelMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_depthPassMatrixLocation = glGetUniformLocation(g_program.program, "u_depthPassMatrix");
    g_diffuseColorLocation = glGetUniformLocation(g_program.program, "u_diffuseColor");
    g_scatterColorLocation = glGetUniformLocation(g_program.program, "u_scatterColor");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");
    g_depthPassTextureLocation = glGetUniformLocation(g_program.program, "u_depthPassTexture");
    g_nearFarLocation = glGetUniformLocation(g_program.program, "u_nearFar");
    g_wrapLocation = glGetUniformLocation(g_program.program, "u_wrap");
    g_scatterWidthLocation = glGetUniformLocation(g_program.program, "u_scatterWidth");
    g_scatterFalloffLocation = glGetUniformLocation(g_program.program, "u_scatterFalloff");


    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    glGenTextures(1, &g_depthPassTexture);
    glBindTexture(GL_TEXTURE_2D, g_depthPassTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, g_depthPassTextureSize, g_depthPassTextureSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    //

    glGenFramebuffers(1, &g_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_depthPassTexture, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

        return GLUS_FALSE;
    }

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //

    // Use a helper function to load an wavefront object file.
    glusShapeLoadWavefront("dragon.obj", &wavefrontObj);

    g_numberVertices = wavefrontObj.numberVertices;

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, wavefrontObj.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) wavefrontObj.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glBufferData(GL_ARRAY_BUFFER, wavefrontObj.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) wavefrontObj.normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&wavefrontObj);

    //

    glUseProgram(g_program.program);

    glusMatrix4x4LookAtf(viewMatrix, g_cameraPosition[0], g_cameraPosition[1], g_cameraPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glusMatrix4x4MultiplyVector3f(lightDirection, viewMatrix, lightDirection);

    glUniform3fv(g_lightDirectionLocation, 1, lightDirection);

    glUniform1i(g_depthPassTextureLocation, 0);

    // Dragon

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_normalLocation);

    //

    glUseProgram(g_programDepthPass.program);

    // Dragon

    glGenVertexArrays(1, &g_vaoDepthPass);
    glBindVertexArray(g_vaoDepthPass);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexDepthPassLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexDepthPassLocation);

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    static GLfloat biasMatrix[] = { 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f };

    GLfloat projectionMatrix[16];

    g_width = width;

    g_height = height;

    //

    glUseProgram(g_programDepthPass.program);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) g_depthPassTextureSize / (GLfloat) g_depthPassTextureSize, g_near, g_far);

    glUniformMatrix4fv(g_projectionMatrixDepthPassLocation, 1, GL_FALSE, projectionMatrix);

    glusMatrix4x4Identityf(g_depthPassMatrix);
    glusMatrix4x4Multiplyf(g_depthPassMatrix, g_depthPassMatrix, biasMatrix);
    glusMatrix4x4Multiplyf(g_depthPassMatrix, g_depthPassMatrix, projectionMatrix);

    //

    glUseProgram(g_program.program);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
    static GLfloat angle = 0.0f;

    GLfloat depthPassMatrix[16];
    GLfloat modelViewMatrix[16];
    GLfloat viewMatrix[16];
    GLfloat modelMatrix[16];
    GLfloat normalMatrix[9];

    // Rendering into the depth pass texture.

    glBindTexture(GL_TEXTURE_2D, 0);

    // Setup for the framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, g_fbo);
    glViewport(0, 0, g_depthPassTextureSize, g_depthPassTextureSize);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    glusMatrix4x4LookAtf(viewMatrix, g_lightPosition[0], g_lightPosition[1], g_lightPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glusMatrix4x4Multiplyf(depthPassMatrix, g_depthPassMatrix, viewMatrix);

    glClear(GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_programDepthPass.program);

    // Render the Dragon.

    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4RotateRzRxRyf(modelMatrix, 0.0f, 0.0f, angle);
    // Upscaling a little bit avoids artifacts.
    glusMatrix4x4Scalef(modelMatrix, 1.05f, 1.05f, 1.05f);
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);

    glUniformMatrix4fv(g_modelViewMatrixDepthPassLocation, 1, GL_FALSE, modelViewMatrix);

    glBindVertexArray(g_vaoDepthPass);

    glDrawArrays(GL_TRIANGLES, 0, g_numberVertices);

    // Revert for the scene.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glViewport(0, 0, g_width, g_height);

    glBindTexture(GL_TEXTURE_2D, g_depthPassTexture);

    //

    // Render the scene.

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_program.program);

    glusMatrix4x4LookAtf(viewMatrix, g_cameraPosition[0], g_cameraPosition[1], g_cameraPosition[2], 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glUniformMatrix4fv(g_viewMatrixLocation, 1, GL_FALSE, viewMatrix);
    glUniformMatrix4fv(g_depthPassMatrixLocation, 1, GL_FALSE, depthPassMatrix);

    // Dragon
    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4RotateRzRxRyf(modelMatrix, 0.0f, 0.0f, angle);
    glusMatrix4x4Multiplyf(modelViewMatrix, viewMatrix, modelMatrix);
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);
    glUniform4f(g_diffuseColorLocation, 0.8f, 0.0f, 0.0f, 1.0f);
    glUniform4f(g_scatterColorLocation, 0.8f, 0.8f, 0.0f, 1.0f);

    glUniform2f(g_nearFarLocation, g_near, g_far);
    glUniform1f(g_wrapLocation, g_wrap);
    glUniform1f(g_scatterWidthLocation, g_scatterWidth);
    glUniform1f(g_scatterFalloffLocation, g_scatterFalloff);

    glBindVertexArray(g_vao);
    glDrawArrays(GL_TRIANGLES, 0, g_numberVertices);

    //

    angle += 20.0f * time;

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glBindTexture(GL_TEXTURE_2D, 0);

    if (g_depthPassTexture)
    {
        glDeleteRenderbuffers(1, &g_depthPassTexture);

        g_depthPassTexture = 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (g_fbo)
    {
        glDeleteFramebuffers(1, &g_fbo);

        g_fbo = 0;
    }

    //

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

    glBindVertexArray(0);

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_vao);

        g_vao = 0;
    }

    glUseProgram(0);

    glusProgramDestroy(&g_program);

    //

    if (g_vaoDepthPass)
    {
        glDeleteVertexArrays(1, &g_vaoDepthPass);

        g_vaoDepthPass = 0;
    }

    glusProgramDestroy(&g_programDepthPass);
}

GLUSvoid key(const GLUSboolean pressed, const GLUSint key)
{
	if (pressed)
	{
		if (key == '1')
		{
			g_wrap -= 0.1f;
		}
		else if (key == '2')
		{
			g_wrap += 0.1f;
		}
		else if (key == '3')
		{
			g_scatterWidth -= 0.1f;
		}
		else if (key == '4')
		{
			g_scatterWidth += 0.1f;
		}
		else if (key == '5')
		{
			g_scatterFalloff -= 5.0f;
		}
		else if (key == '6')
		{
			g_scatterFalloff += 5.0f;
		}
	}

	g_wrap = glusMathClampf(g_wrap, 0.0f, 1.0f);
	g_scatterWidth = glusMathClampf(g_scatterWidth, 0.0f, 1.0f);
	g_scatterFalloff = glusMathClampf(g_scatterFalloff, 0.0f, 50.0f);
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

    glusWindowSetKeyFunc(key);

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

    if (!glusWindowCreate("GLUS Example Window", 640, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    // Print out the keys
    printf("Keys:\n");
    printf("1       = Decrease wrap\n");
    printf("2       = Increase wrap\n");
    printf("3       = Decrease scatter width\n");
    printf("4       = Increase scatter width\n");
    printf("5       = Decrease scatter falloff\n");
    printf("6       = Increase scatter falloff\n");
    printf("\n");

    glusWindowRun();

    return 0;
}
