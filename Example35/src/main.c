/**
 * OpenGL 3 - Example 35
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

// Number of peel layers. 8 is enough to peel completely the dragon.
#define LAYERS	8

/**
 * Properties of the light.
 */
struct LightProperties
{
    GLfloat direction[3];
    GLfloat ambientColor[4];
    GLfloat diffuseColor[4];
    GLfloat specularColor[4];
};

/**
 * Properties of the material, basically all the color factors without the emissive color component.
 */
struct MaterialProperties
{
    GLfloat ambientColor[4];
    GLfloat diffuseColor[4];
    GLfloat specularColor[4];
    GLfloat specularExponent;
    GLfloat alpha;
};

/**
 * Locations for the light properties.
 */
struct LightLocations
{
    GLint directionLocation;
    GLint ambientColorLocation;
    GLint diffuseColorLocation;
    GLint specularColorLocation;
};

/**
 * Locations for the material properties.
 */
struct MaterialLocations
{
    GLint ambientColorLocation;
    GLint diffuseColorLocation;
    GLint specularColorLocation;
    GLint specularExponentLocation;
    GLint alphaLocation;
};

static GLfloat g_viewMatrix[16];

static GLUSprogram g_program;

static GLint g_biasMatrixLocation;

static GLint g_projectionMatrixLocation;

static GLint g_modelViewMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLint g_peelTextureLocation;

static GLint g_layerLocation;

static struct LightLocations g_light;

static struct MaterialLocations g_material;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_vao;

static GLuint g_numberVertices;

//

static GLuint g_colorTexture;

static GLuint g_depthTexture[2];

static GLuint g_blendFullscreenFBO;

static GLuint g_blendFullscreenVAO;

//

static GLUSprogram g_blendFullscreenProgram;

static GLint g_framebufferTextureBlendFullscreenLocation;

static GLint g_layersBlendFullscreenLocation;

GLUSboolean init(GLUSvoid)
{
    // This is a white light.
    struct LightProperties light = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

    // Green color material with white specular color, half transparent.
    struct MaterialProperties material = { { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 20.0f, 0.5f };

    static GLfloat biasMatrix[] = { 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 0.5f, 0.5f, 1.0f };

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape wavefrontObj;

    glusFileLoadText("../Example35/shader/phong_depth_peel.vert.glsl", &vertexSource);
    glusFileLoadText("../Example35/shader/phong_depth_peel.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_biasMatrixLocation = glGetUniformLocation(g_program.program, "u_biasMatrix");
    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_modelViewMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");

    g_light.directionLocation = glGetUniformLocation(g_program.program, "u_light.direction");
    g_light.ambientColorLocation = glGetUniformLocation(g_program.program, "u_light.ambientColor");
    g_light.diffuseColorLocation = glGetUniformLocation(g_program.program, "u_light.diffuseColor");
    g_light.specularColorLocation = glGetUniformLocation(g_program.program, "u_light.specularColor");

    g_material.ambientColorLocation = glGetUniformLocation(g_program.program, "u_material.ambientColor");
    g_material.diffuseColorLocation = glGetUniformLocation(g_program.program, "u_material.diffuseColor");
    g_material.specularColorLocation = glGetUniformLocation(g_program.program, "u_material.specularColor");
    g_material.specularExponentLocation = glGetUniformLocation(g_program.program, "u_material.specularExponent");
    g_material.alphaLocation = glGetUniformLocation(g_program.program, "u_material.alpha");

    g_peelTextureLocation = glGetUniformLocation(g_program.program, "u_peelTexture");
    g_layerLocation = glGetUniformLocation(g_program.program, "u_layer");


    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

	glusFileLoadText("../Example35/shader/fullscreen_blend.vert.glsl", &vertexSource);
	glusFileLoadText("../Example35/shader/fullscreen_blend.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_blendFullscreenProgram, (const GLchar**)&vertexSource.text, 0, 0, 0, (const GLchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	//

	g_framebufferTextureBlendFullscreenLocation = glGetUniformLocation(g_blendFullscreenProgram.program, "u_framebufferTexture");
	g_layersBlendFullscreenLocation = glGetUniformLocation(g_blendFullscreenProgram.program, "u_layers");

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

	//
	// Setting up the frame buffer.
	//

	glGenTextures(1, &g_colorTexture);
	glActiveTexture(GL_TEXTURE0);

	glBindTexture(GL_TEXTURE_2D_ARRAY, g_colorTexture);

	glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, SCREEN_WIDTH, SCREEN_HEIGHT, LAYERS, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	//

	glGenTextures(2, g_depthTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, g_depthTexture[0]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    // Peel depth test "function". See shader for more information.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);

	glBindTexture(GL_TEXTURE_2D, g_depthTexture[1]);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    // Peel depth test "function". See shader for more information.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_GREATER);

	glBindTexture(GL_TEXTURE_2D, 0);

	//

	glGenFramebuffers(1, &g_blendFullscreenFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, g_blendFullscreenFBO);

	// Attach the color buffer ...
	glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, g_colorTexture, 0, 0);

	// ... and the depth buffer,
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_depthTexture[1], 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

		return GLUS_FALSE;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

    //

	glUseProgram(g_blendFullscreenProgram.program);

	glGenVertexArrays(1, &g_blendFullscreenVAO);
	glBindVertexArray(g_blendFullscreenVAO);

	glUniform1i(g_framebufferTextureBlendFullscreenLocation, 0);
	glUniform1i(g_layersBlendFullscreenLocation, LAYERS);

    glBindVertexArray(0);

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

    glBindVertexArray(0);

    //

    glusMatrix4x4LookAtf(g_viewMatrix, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    //

    glusVector3Normalizef(light.direction);

    // Transform light to camera space, as it is currently in world space.
    glusMatrix4x4MultiplyVector3f(light.direction, g_viewMatrix, light.direction);

    // Set up light ...
    glUniform3fv(g_light.directionLocation, 1, light.direction);
    glUniform4fv(g_light.ambientColorLocation, 1, light.ambientColor);
    glUniform4fv(g_light.diffuseColorLocation, 1, light.diffuseColor);
    glUniform4fv(g_light.specularColorLocation, 1, light.specularColor);

    // ... and material values.
    glUniform4fv(g_material.ambientColorLocation, 1, material.ambientColor);
    glUniform4fv(g_material.diffuseColorLocation, 1, material.diffuseColor);
    glUniform4fv(g_material.specularColorLocation, 1, material.specularColor);
    glUniform1f(g_material.specularExponentLocation, material.specularExponent);
    glUniform1f(g_material.alphaLocation, material.alpha);

	glUniform1i(g_peelTextureLocation, 1);

	glUniformMatrix4fv(g_biasMatrixLocation, 1, GL_FALSE, biasMatrix);

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    GLfloat projectionMatrix[16];

    glViewport(0, 0, width, height);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    // Just pass the projection matrix. The final matrix is calculated in the shader.
    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
    static GLfloat angle = 0.0f;

    GLint peelLayer, defaultDepth, peelDepth;

    GLfloat modelViewMatrix[16];
    GLfloat normalMatrix[9];

    glusMatrix4x4Identityf(modelViewMatrix);

    glusMatrix4x4RotateRyf(modelViewMatrix, angle);

    glusMatrix4x4Multiplyf(modelViewMatrix, g_viewMatrix, modelViewMatrix);

    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUseProgram(g_program.program);
    glBindVertexArray(g_vao);

    glUniformMatrix4fv(g_modelViewMatrixLocation, 1, GL_FALSE, modelViewMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    // Depth peeling passes.
	// see http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.18.9286&rep=rep1&type=pdf

    // Disable color texture ...
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	for (peelLayer = 0; peelLayer < LAYERS; peelLayer++)
	{
	    glUniform1i(g_layerLocation, peelLayer);

		peelDepth = peelLayer % 2;
		defaultDepth = (peelLayer + 1) % 2;

		// ... and activate the peeling depth texture.
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, g_depthTexture[peelDepth]);

		glBindFramebuffer(GL_FRAMEBUFFER, g_blendFullscreenFBO);

		// Now color buffer ...
		glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, g_colorTexture, 0, peelLayer);

		// ... and default depth buffer can be used as the frame buffer.
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, g_depthTexture[defaultDepth], 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));

			return GLUS_FALSE;
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glDrawArrays(GL_TRIANGLES, 0, g_numberVertices);
	}

    // Fullscreen quad rendering.

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, g_colorTexture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glUseProgram(g_blendFullscreenProgram.program);
    glBindVertexArray(g_blendFullscreenVAO);

	glDisable(GL_DEPTH_TEST);

	// Blending is happening in the shader.
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glEnable(GL_DEPTH_TEST);

    angle += 30.0f * time;

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

    glBindVertexArray(0);

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_vao);

        g_vao = 0;
    }

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_blendFullscreenVAO);

        g_vao = 0;
    }

    glUseProgram(0);

    glusProgramDestroy(&g_program);

    glusProgramDestroy(&g_blendFullscreenProgram);

    //

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

	if (g_colorTexture)
	{
		glDeleteTextures(1, &g_colorTexture);

		g_colorTexture = 0;
	}

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_depthTexture[0])
	{
		glDeleteTextures(1, &g_depthTexture[0]);

		g_depthTexture[0] = 0;
	}

	if (g_depthTexture[1])
	{
		glDeleteTextures(1, &g_depthTexture[1]);

		g_depthTexture[1] = 0;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	if (g_blendFullscreenFBO)
	{
		glDeleteFramebuffers(1, &g_blendFullscreenFBO);

		g_blendFullscreenFBO = 0;
	}
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

    if (!glusWindowCreate("GLUS Example Window", SCREEN_WIDTH, SCREEN_HEIGHT, GLUS_FALSE, GLUS_TRUE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
