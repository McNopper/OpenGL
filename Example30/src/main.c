/**
 * OpenGL 4 - Example 30
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

#define WIDTH 640
#define HEIGHT 480

#define DIRECTION_BUFFER_PADDING 1

#define PADDING_VALUE -321.123f

// Every ray can have two sub rays (reflect and refract). This can be organized as a tree, with  breadth-first indexing.
// So, a tree with a depth has 2^depth-1 nodes. In this case we have a MAX_DEPTH of 5
#define NUM_STACK_NODES (2 * 2 * 2 * 2 * 2 - 1)

// As no recursion is possible in GLSL, the following is done:
// As the number of rays (= nodes) is known, all rays plus sub rays are executed. All needed values are stored in a stack node.
// After this is done, the tree is traversed again from the leaf node to the root. Now the color of node can be calculated
// by the using the sub nodes. Finally, in the root node the final color is stored.
#define STACK_NODE_FLOATS (4 + (3 + 1) + 4 + 4 + (3 + 1) + 4 + 1 + 1 + 1 + 1)

#define NUM_SPHERES 6
#define NUM_LIGHTS 1

/**
 * The used shader program.
 */
static GLUSprogram g_program;

/**
 * The location of the texture in the shader program.
 */
static GLint g_textureLocation;

/**
 * The empty VAO.
 */
static GLuint g_vao;

/**
 * The texture.
 */
static GLuint g_texture;

/**
 * Size of the work goups.
 */
static GLuint g_localSize = 16;

//

/**
 * The used compute shader program.
 */
static GLUSprogram g_computeProgram;

//
// Shader Storage Buffer Objects
//

static GLuint g_directionSSBO;

static GLfloat g_directionBuffer[WIDTH * HEIGHT * (3 + DIRECTION_BUFFER_PADDING)];

//

static GLuint g_positionSSBO;

static GLfloat g_positionBuffer[WIDTH * HEIGHT * 4];

//

static GLuint g_stackSSBO;

static GLfloat g_stackBuffer[WIDTH * HEIGHT * STACK_NODE_FLOATS * NUM_STACK_NODES];

//

typedef struct _Material
{
	GLfloat emissiveColor[4];

	GLfloat diffuseColor[4];

	GLfloat specularColor[4];

	GLfloat shininess;

	GLfloat alpha;

	GLfloat reflectivity;

	GLfloat padding;

} Material;

typedef struct _Sphere
{
	GLfloat center[4];

	GLfloat radius;

	GLfloat padding[3];

	Material material;

} Sphere;

static GLuint g_sphereSSBO;

Sphere g_sphereBuffer[NUM_SPHERES] = {
		// Ground sphere
		{ { 0.0f, -10001.0f, -20.0f, 1.0f }, 10000.0f, {PADDING_VALUE, PADDING_VALUE, PADDING_VALUE}, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.4f, 0.4f, 0.4f, 1.0f }, { 0.0f, 0.0f, 0.0f, 1.0f }, 0.0f, 1.0f, 0.0f, PADDING_VALUE } },
		// Transparent sphere
		{ { 0.0f, 0.0f, -10.0f, 1.0f }, 1.0f, {PADDING_VALUE, PADDING_VALUE, PADDING_VALUE}, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f, 0.2f, 1.0f, PADDING_VALUE } },
		// Reflective sphere
		{ { 1.0f, -0.75f, -7.0f, 1.0f }, 0.25f, {PADDING_VALUE, PADDING_VALUE, PADDING_VALUE}, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f, 1.0f, 0.8f, PADDING_VALUE } },
		// Blue sphere
		{ { 2.0f, 1.0f, -16.0f, 1.0f }, 2.0f, {PADDING_VALUE, PADDING_VALUE, PADDING_VALUE}, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 0.8f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f, 1.0f, 0.2f, PADDING_VALUE } },
		// Green sphere
		{ { -2.0f, 0.25f, -6.0f, 1.0f }, 1.25f, {PADDING_VALUE, PADDING_VALUE, PADDING_VALUE}, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.0f, 0.8f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f, 1.0f, 0.2f, PADDING_VALUE } },
		// Red sphere
		{ { 3.0f, 0.0f, -8.0f, 1.0f }, 1.0f, {PADDING_VALUE, PADDING_VALUE, PADDING_VALUE}, { { 0.0f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.0f, 0.0f, 1.0f }, { 0.8f, 0.8f, 0.8f, 1.0f }, 20.0f, 1.0f, 0.2f, PADDING_VALUE } }
};

//

typedef struct _PointLight
{
	GLfloat position[4];

	GLfloat color[4];

} PointLight;

static GLuint g_pointLightSSBO;

PointLight g_lightBuffer[NUM_LIGHTS] = {
		{{0.0f, 5.0f, -5.0f, 1.0f}, { 1.0f, 1.0f, 1.0f, 1.0f }}
};

GLUSboolean init(GLUSvoid)
{
	GLint i;

	GLUStextfile vertexSource;
	GLUStextfile fragmentSource;
	GLUStextfile computeSource;

	glusFileLoadText("../Example30/shader/fullscreen.vert.glsl", &vertexSource);
	glusFileLoadText("../Example30/shader/texture.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_program, (const GLchar**)&vertexSource.text, 0, 0, 0, (const GLchar**)&fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&fragmentSource);

	glusFileLoadText("../Example30/shader/raytrace.comp.glsl", &computeSource);

	glusProgramBuildComputeFromSource(&g_computeProgram, (const GLchar**)&computeSource.text);

	glusFileDestroyText(&computeSource);

	//

	// Retrieve the uniform locations in the program.
	g_textureLocation = glGetUniformLocation(g_program.program, "u_texture");

	//

	// Generate and bind a texture.
	glGenTextures(1, &g_texture);
	glBindTexture(GL_TEXTURE_2D, g_texture);

	// Create an empty image.
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

	// Setting the texture parameters.
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glBindTexture(GL_TEXTURE_2D, 0);

	//
	//

	glUseProgram(g_program.program);

	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);

	//

	// Also bind created texture ...
	glBindTexture(GL_TEXTURE_2D, g_texture);

	// ... and bind this texture as an image, as we will write to it. see binding = 0 in shader.
	glBindImageTexture(0, g_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA8);

	// ... and as this is texture number 0, bind the uniform to the program.
	glUniform1i(g_textureLocation, 0);

	//
	//

	glUseProgram(g_computeProgram.program);

	//

	printf("Preparing buffers ... ");

	// Generate the ray directions depending on FOV, width and height.
	if (!glusRaytracePerspectivef(g_directionBuffer, DIRECTION_BUFFER_PADDING, 30.0f, WIDTH, HEIGHT))
	{
		printf("failed!\n");

		printf("Error: Could not create direction buffer.\n");

		return GLUS_FALSE;
	}

	// Compute shader will use these textures just for input.
	glusRaytraceLookAtf(g_positionBuffer, g_directionBuffer, g_directionBuffer, DIRECTION_BUFFER_PADDING, WIDTH, HEIGHT, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);

	for (i = 0; i < WIDTH * HEIGHT * STACK_NODE_FLOATS * NUM_STACK_NODES; i++)
	{
		g_stackBuffer[i] = 0.0f;
	}

	printf("done!\n");

	//
	// Buffers with the initial ray position and direction.
	//

	glGenBuffers(1, &g_directionSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_directionSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, WIDTH * HEIGHT * (3 + DIRECTION_BUFFER_PADDING) * sizeof(GLfloat), g_directionBuffer, GL_STATIC_DRAW);
	// see binding = 1 in the shader
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_directionSSBO);

	//

	glGenBuffers(1, &g_positionSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_positionSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, WIDTH * HEIGHT * 4 * sizeof(GLfloat), g_positionBuffer, GL_STATIC_DRAW);
	// see binding = 2 in the shader
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_positionSSBO);

	//

	glGenBuffers(1, &g_stackSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_stackSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, WIDTH * HEIGHT * STACK_NODE_FLOATS * NUM_STACK_NODES * sizeof(GLfloat), g_stackBuffer, GL_STATIC_DRAW);
	// see binding = 3 in the shader
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, g_stackSSBO);

	//

	glGenBuffers(1, &g_sphereSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_sphereSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_SPHERES * sizeof(Sphere), g_sphereBuffer, GL_STATIC_DRAW);
	// see binding = 4 in the shader
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, g_sphereSSBO);

	//

	glGenBuffers(1, &g_pointLightSSBO);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_pointLightSSBO);
	glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_LIGHTS * sizeof(PointLight), g_lightBuffer, GL_STATIC_DRAW);
	// see binding = 5 in the shader
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, g_pointLightSSBO);

	//

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	glViewport(0, 0, width, height);
}

GLUSboolean update(GLUSfloat time)
{
	// Switch to the compute shader.
	glUseProgram(g_computeProgram.program);

	// Create threads depending on width, height and block size. In this case we have 1200 threads.
	glDispatchCompute(WIDTH / g_localSize, HEIGHT / g_localSize, 1);

	// Switch back to the render program.
	glUseProgram(g_program.program);

	// Here we full screen the output of the compute shader.
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
	glBindTexture(GL_TEXTURE_2D, 0);

	if (g_texture)
	{
		glDeleteTextures(1, &g_texture);

		g_texture = 0;
	}

	//

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (g_directionSSBO)
	{
		glDeleteBuffers(1, &g_directionSSBO);

		g_directionSSBO = 0;
	}

	if (g_positionSSBO)
	{
		glDeleteBuffers(1, &g_positionSSBO);

		g_positionSSBO = 0;
	}

	if (g_stackSSBO)
	{
		glDeleteBuffers(1, &g_stackSSBO);

		g_stackSSBO = 0;
	}

	if (g_sphereSSBO)
	{
		glDeleteBuffers(1, &g_sphereSSBO);

		g_sphereSSBO = 0;
	}

	if (g_pointLightSSBO)
	{
		glDeleteBuffers(1, &g_pointLightSSBO);

		g_pointLightSSBO = 0;
	}

	//

	glBindVertexArray(0);

	if (g_vao)
	{
		glDeleteVertexArrays(1, &g_vao);

		g_vao = 0;
	}

	glUseProgram(0);

	glusProgramDestroy(&g_program);

	glusProgramDestroy(&g_computeProgram);
}

int main(int argc, char* argv[])
{
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
    		EGL_NONE
    };

    glusWindowSetInitFunc(init);

    glusWindowSetReshapeFunc(reshape);

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

    // Again, makes programming for this example easier.
    if (!glusWindowCreate("GLUS Example Window", WIDTH, HEIGHT, GLUS_FALSE, GLUS_TRUE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
