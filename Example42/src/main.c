/**
 * OpenGL 4 - Example 42
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

#define MAX_MUL_REDUCE_RECIPROCAL 256.0f
#define MAX_MIN_REDUCE_RECIPROCAL 512.0f
#define MAX_MAX_SPAN 16.0f

static GLfloat g_viewMatrix[16];

//

static GLUSprogram g_program;

static GLint g_texelStepLocation;
static GLint g_showEdgesLocation;
static GLint g_fxaaOnLocation;

static GLint g_lumaThresholdLocation;
static GLint g_mulReduceLocation;
static GLint g_minReduceLocation;
static GLint g_maxSpanLocation;

//

static GLuint g_vao;

//

static GLuint g_colorTexture;

static GLuint g_depthRenderbuffer;

static GLuint g_colorDepthFBO;

//

static struct LightProperties g_light = { { 1.0f, 1.0f, 1.0f }, { 0.2f, 0.2f, 0.2f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f } };

static struct CameraProperties g_camera = { {0.0f, 0.0f, 10.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};

//

static GLboolean g_animationOn = GL_FALSE;

static GLint g_showEdges = 0;
static GLint g_fxaaOn = 1;

static GLfloat g_lumaThreshold = 0.5f;
static GLfloat g_mulReduceReciprocal = 8.0f;
static GLfloat g_minReduceReciprocal = 128.0f;
static GLfloat g_maxSpan = 8.0f;

GLUSboolean init(GLUSvoid)
{
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    //

    glusMatrix4x4LookAtf(g_viewMatrix, g_camera.eye[0], g_camera.eye[1], g_camera.eye[2], g_camera.center[0], g_camera.center[1], g_camera.center[2], g_camera.up[0], g_camera.up[1], g_camera.up[2]);

    //

    if (!initWavefront(g_viewMatrix, &g_light))
    {
    	return GLUS_FALSE;
    }

    //

    glusFileLoadText("../Example42/shader/fxaa.vert.glsl", &vertexSource);
    glusFileLoadText("../Example42/shader/fxaa.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    g_texelStepLocation = glGetUniformLocation(g_program.program, "u_texelStep");
    g_showEdgesLocation = glGetUniformLocation(g_program.program, "u_showEdges");
    g_fxaaOnLocation = glGetUniformLocation(g_program.program, "u_fxaaOn");

    g_lumaThresholdLocation = glGetUniformLocation(g_program.program, "u_lumaThreshold");
    g_mulReduceLocation = glGetUniformLocation(g_program.program, "u_mulReduce");
    g_minReduceLocation = glGetUniformLocation(g_program.program, "u_minReduce");
    g_maxSpanLocation = glGetUniformLocation(g_program.program, "u_maxSpan");

    //
    // Setting up the offscreen frame buffer.
    //

    glGenTextures(1, &g_colorTexture);
    glBindTexture(GL_TEXTURE_2D, g_colorTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GLUS_RGB, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GLUS_RGB, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_2D, 0);

    //

    glGenRenderbuffers(1, &g_depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, g_depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCREEN_WIDTH, SCREEN_HEIGHT);

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    //

    glGenFramebuffers(1, &g_colorDepthFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, g_colorDepthFBO);

    // Attach the color buffer ...
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_colorTexture, 0);

    // ... and the depth buffer,
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_depthRenderbuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

        return GLUS_FALSE;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //
    // VAO with no elements.
    //

    glUseProgram(g_program.program);

    glUniform2f(g_texelStepLocation, 1.0f / (GLfloat)SCREEN_WIDTH, 1.0f / (GLfloat)SCREEN_HEIGHT);

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glBindVertexArray(0);

    glUseProgram(g_program.program);

    //

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	GLfloat projectionMatrix[16];

    glViewport(0, 0, width, height);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 1000.0f);

    glBindFramebuffer(GL_FRAMEBUFFER, g_colorDepthFBO);

    glViewport(0, 0, width, height);

    reshapeWavefront(projectionMatrix);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GLUSboolean update(GLUSfloat time)
{
	if (!g_animationOn)
	{
		time = 0.0f;
	}

	//
    // Off screen rendering of the scene.
	//

    glBindFramebuffer(GL_FRAMEBUFFER, g_colorDepthFBO);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (!updateWavefront(time))
    {
    	return GLUS_FALSE;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //
    // Rendering the off screen color buffer using FXAA.
    //

    glBindTexture(GL_TEXTURE_2D, g_colorTexture);

    glDisable(GL_DEPTH_TEST);

    glUseProgram(g_program.program);

    glUniform1i(g_showEdgesLocation, g_showEdges);
    glUniform1i(g_fxaaOnLocation, g_fxaaOn);

    glUniform1f(g_lumaThresholdLocation, g_lumaThreshold);
    glUniform1f(g_mulReduceLocation, 1.0f / g_mulReduceReciprocal);
    glUniform1f(g_minReduceLocation, 1.0f / g_minReduceReciprocal);
    glUniform1f(g_maxSpanLocation, g_maxSpan);

    glBindVertexArray(g_vao);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindVertexArray(0);

    glUseProgram(0);

    glEnable(GL_DEPTH_TEST);

    glBindTexture(GL_TEXTURE_2D, 0);

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glBindVertexArray(0);

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_vao);

        g_vao = 0;
    }

    glUseProgram(0);

    glusProgramDestroy(&g_program);

    //

    glBindTexture(GL_TEXTURE_2D, 0);

    if (g_colorTexture)
    {
        glDeleteTextures(1, &g_colorTexture);

        g_colorTexture = 0;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    if (g_depthRenderbuffer)
    {
        glDeleteRenderbuffers(1, &g_depthRenderbuffer);

        g_depthRenderbuffer = 0;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (g_colorDepthFBO)
	{
		glDeleteFramebuffers(1, &g_colorDepthFBO);

		g_colorDepthFBO = 0;
	}

    terminateWavefront();
}

GLUSvoid key(const GLUSboolean pressed, const GLUSint key)
{
	if (pressed)
	{
		if (key == '1')
		{
			g_lumaThreshold -= 0.05f;
		}
		else if (key == '2')
		{
			g_lumaThreshold += 0.05f;
		}
		else if (key == '3')
		{
			g_mulReduceReciprocal *= 2.0f;
		}
		else if (key == '4')
		{
			g_mulReduceReciprocal /= 2.0f;
		}
		else if (key == '5')
		{
			g_minReduceReciprocal *= 2.0f;
		}
		else if (key == '6')
		{
			g_minReduceReciprocal /= 2.0f;
		}
		else if (key == '7')
		{
			g_maxSpan -= 1.0f;
		}
		else if (key == '8')
		{
			g_maxSpan += 1.0f;
		}
		else if (key == 'a')
		{
			g_animationOn = !g_animationOn;
		}
		else if (key == 'e')
		{
			g_showEdges = !g_showEdges;
		}
		else if (key == ' ')
		{
			g_fxaaOn = !g_fxaaOn;
		}

		g_lumaThreshold = glusMathClampf(g_lumaThreshold, 0.0f, 1.0f);

		g_mulReduceReciprocal = glusMathClampf(g_mulReduceReciprocal, 1.0f, MAX_MUL_REDUCE_RECIPROCAL);

		g_minReduceReciprocal = glusMathClampf(g_minReduceReciprocal, 1.0f, MAX_MIN_REDUCE_RECIPROCAL);

		g_maxSpan = glusMathClampf(g_maxSpan, 1.0f, MAX_MAX_SPAN);
	}
}

int main(int argc, char* argv[])
{
	// No depth buffer, as we finally render a full screen rectangle.
	// No MSAA, as we use FXAA.
	EGLint eglConfigAttributes[] = {
	        EGL_RED_SIZE, 8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE, 8,
	        EGL_DEPTH_SIZE, 0,
	        EGL_STENCIL_SIZE, 0,
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

    glusWindowSetKeyFunc(key);

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

    // Fixed, non re-sizable screen size for simpler example code.
    if (!glusWindowCreate("GLUS Example Window", SCREEN_WIDTH, SCREEN_HEIGHT, GLUS_FALSE, GLUS_TRUE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    // Print out the keys
    printf("Keys:\n");
    printf("1       = Decrease luma threshold\n");
    printf("2       = Increase luma threshold\n");
    printf("3       = Decrease multiplication reduce\n");
    printf("4       = Increase multiplication reduce\n");
    printf("5       = Decrease minimum reduce\n");
    printf("6       = Increase minimum reduce\n");
    printf("7       = Decrease maximum span\n");
    printf("8       = Increase maximum span\n");
    printf("a       = Animation on/off\n");
    printf("e       = Show edges on/off\n");
    printf("[Space]	= FXAA on/off\n");
    printf("\n");

    glusWindowRun();

    return 0;
}
