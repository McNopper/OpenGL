/**
 * OpenGL 4 - Example 36
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

// Maximum number of nodes in the linked list. Shared for all fragments.
#define MAX_NODES (8*SCREEN_WIDTH*SCREEN_HEIGHT)

#define BINDING_ATOMIC_FREE_INDEX 0
#define BINDING_IMAGE_HEAD_INDEX 0
#define BINDING_BUFFER_LINKED_LIST 0

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

static GLint g_projectionMatrixLocation;

static GLint g_modelViewMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_maxNodesLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static struct LightLocations g_light;

static struct MaterialLocations g_material;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_vao;

static GLuint g_numberVertices;

//

static GLuint g_blendFullscreenVAO;

//

static GLUSprogram g_blendFullscreenProgram;

//

static GLuint g_freeNodeIndex;

static GLuint g_headIndexTexture;

static GLuint g_clearBuffer;

static GLuint g_linkedListBuffer;

GLUSboolean init(GLUSvoid)
{
    // This is a white light.
    struct LightProperties light = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

    // Green color material with white specular color, half transparent.
    struct MaterialProperties material = { { 0.0f, 1.0f, 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 20.0f, 0.5f };

    // Buffer for cleaning the head index testure.
    static GLuint clearBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape wavefrontObj;

    GLuint i;

    for (i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++)
    {
    	// 0xffffffff means end of list, so for the start tehre is no entry.
    	clearBuffer[i] = 0xffffffff;
    }

    //

    glusFileLoadText("../Example36/shader/phong_linked_list.vert.glsl", &vertexSource);
    glusFileLoadText("../Example36/shader/phong_linked_list.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

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

    g_maxNodesLocation = glGetUniformLocation(g_program.program, "u_maxNodes");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

	glusFileLoadText("../Example36/shader/fullscreen_blend.vert.glsl", &vertexSource);
	glusFileLoadText("../Example36/shader/fullscreen_blend.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_blendFullscreenProgram, (const GLchar**)&vertexSource.text, 0, 0, 0, (const GLchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	// Atomic counter to gather a free node slot concurrently.

	glGenBuffers(1, &g_freeNodeIndex);

	glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, BINDING_ATOMIC_FREE_INDEX, g_freeNodeIndex);
	glBufferData(GL_ATOMIC_COUNTER_BUFFER, sizeof(GLuint), 0, GL_DYNAMIC_DRAW);

	// Head index texture/image, which contains the

	glGenTextures(1, &g_headIndexTexture);

	glBindTexture(GL_TEXTURE_2D, g_headIndexTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, SCREEN_WIDTH, SCREEN_HEIGHT, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindImageTexture(BINDING_IMAGE_HEAD_INDEX, g_headIndexTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

	// Buffer to clear/reset the head pointers.

	glGenBuffers(1, &g_clearBuffer);

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, g_clearBuffer);
	glBufferData(GL_PIXEL_UNPACK_BUFFER, SCREEN_WIDTH * SCREEN_HEIGHT * sizeof(GLuint), clearBuffer, GL_STATIC_COPY);

	// Buffer for the linked list.

	glGenBuffers(1, &g_linkedListBuffer);

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_LINKED_LIST, g_linkedListBuffer);
	// Size is RGBA, depth (5 * GLfloat), next pointer (1 * GLuint) and 2 paddings (2 * GLfloat).
	glBufferData(GL_SHADER_STORAGE_BUFFER, MAX_NODES * (sizeof(GLfloat) * 5 + sizeof(GLuint) * 1) + sizeof(GLfloat) * 2, 0, GL_DYNAMIC_DRAW);

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

	glUseProgram(g_blendFullscreenProgram.program);

	glGenVertexArrays(1, &g_blendFullscreenVAO);
	glBindVertexArray(g_blendFullscreenVAO);

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

    glUniform1ui(g_maxNodesLocation, MAX_NODES);

    //

    glDisable(GL_DEPTH_TEST);

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

    static GLuint zero = 0;

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

    //
    // Linked list rendering pass.
    //

    // Reset the atomic counter.
    glBufferSubData(GL_ATOMIC_COUNTER_BUFFER, 0, sizeof(GLuint), &zero);

    // Reset the head pointers by copying the clear buffer into the texture.
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, g_clearBuffer);

    glBindTexture(GL_TEXTURE_2D, g_headIndexTexture);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

    //

	glDrawArrays(GL_TRIANGLES, 0, g_numberVertices);

	//
    // Fullscreen quad rendering.
	//

    glUseProgram(g_blendFullscreenProgram.program);
    glBindVertexArray(g_blendFullscreenVAO);

	// Resolving and blending is happening in the shader.
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
    //
    //

	glBindBuffer(GL_ATOMIC_COUNTER_BUFFER, 0);

	if (g_freeNodeIndex)
	{
		glDeleteBuffers(1, &g_freeNodeIndex);

		g_freeNodeIndex = 0;
	}

	//

	glBindTexture(GL_TEXTURE_2D, 0);

	glBindImageTexture(BINDING_IMAGE_HEAD_INDEX, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_R32UI);

	if (g_headIndexTexture)
	{
		glDeleteTextures(1, &g_headIndexTexture);

		g_headIndexTexture = 0;
	}

	//

	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, 0);

	if (g_clearBuffer)
	{
		glDeleteBuffers(1, &g_clearBuffer);

		g_clearBuffer = 0;
	}

	//

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (g_linkedListBuffer)
	{
		glDeleteBuffers(1, &g_linkedListBuffer);

		g_linkedListBuffer = 0;
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
    		EGL_CONTEXT_MAJOR_VERSION, 4,
    		EGL_CONTEXT_MINOR_VERSION, 4,
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
