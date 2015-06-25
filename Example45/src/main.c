/**
 * OpenGL 4 - Example 45
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

#define LOCATION_VERTEX 0
#define LOCATION_TEXCOORD 1
#define BINDING_VOXEL_GRID 0
#define BINDING_DIFFUSE 0

#define WINDOW_SIZE 512

// Size of the 3D texture.
#define VOXEL_GRID_SIZE 128

// Values, that the model is inside the voxel grid.
#define EYE ((GLfloat)VOXEL_GRID_SIZE * 0.5f)
#define NEAR 0.0f
#define FAR ((GLfloat)VOXEL_GRID_SIZE)

//

static GLUSprogram g_program;

static GLint g_projectionMatrixLocation;
static GLint g_viewMatrixLocation;
static GLint g_modelMatrixLocation;

static GLint g_halfPixelSizeLocation;

//

static GLfloat g_projectionMatrix[16];
static GLfloat g_viewMatrix[16];
static GLfloat g_modelMatrix[16];

//

static GLUSwavefront g_wavefront;

static GLuint g_voxelGrid;

//
// Fullscreen variables.
//

static GLUSprogram g_fullscreenProgram;

static GLuint g_fullscreenVao;

GLUSboolean init(GLUSvoid)
{
    GLUStextfile vertexSource;
    GLUStextfile geometrySource;
    GLUStextfile fragmentSource;

	GLUStgaimage image;

	GLUSgroupList* groupWalker;
	GLUSmaterialList* materialWalker;

	GLfloat halfPixelSize[2];

    glusFileLoadText("../Example45/shader/voxelize.vert.glsl", &vertexSource);
    glusFileLoadText("../Example45/shader/voxelize.geom.glsl", &geometrySource);
    glusFileLoadText("../Example45/shader/voxelize.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, (const GLUSchar**) &geometrySource.text, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&geometrySource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_viewMatrixLocation = glGetUniformLocation(g_program.program, "u_viewMatrix");
    g_modelMatrixLocation = glGetUniformLocation(g_program.program, "u_modelMatrix");

    g_halfPixelSizeLocation = glGetUniformLocation(g_program.program, "u_halfPixelSize");

    //
    // 3D model
    //

	glusWavefrontLoad("ChessKing.obj", &g_wavefront);

	glGenBuffers(1, &g_wavefront.verticesVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.verticesVBO);
	glBufferData(GL_ARRAY_BUFFER, g_wavefront.numberVertices * 4 * sizeof(GLfloat), (GLfloat*)g_wavefront.vertices, GL_STATIC_DRAW);

	glGenBuffers(1, &g_wavefront.texCoordsVBO);
	glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.texCoordsVBO);
	glBufferData(GL_ARRAY_BUFFER, g_wavefront.numberVertices * 2 * sizeof(GLfloat), (GLfloat*)g_wavefront.texCoords, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	//
	// Set up indices and the VAOs for each group
	//

	glUseProgram(g_program.program);

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
		glGenBuffers(1, &groupWalker->group.indicesVBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.numberIndices * sizeof(GLuint), (GLuint*)groupWalker->group.indices, GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		//

		glGenVertexArrays(1, &groupWalker->group.vao);
		glBindVertexArray(groupWalker->group.vao);

		glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.verticesVBO);
		glVertexAttribPointer(LOCATION_VERTEX, 4, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(LOCATION_VERTEX);

		glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.texCoordsVBO);
		glVertexAttribPointer(LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(LOCATION_TEXCOORD);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);

		glBindVertexArray(0);

		groupWalker = groupWalker->next;
	}

	//
	// Load the textures, if there are available
	//

	glActiveTexture(GL_TEXTURE1);

	materialWalker = g_wavefront.materials;
	while (materialWalker)
	{
		if (materialWalker->material.diffuseTextureFilename[0] != '\0')
		{
			// Load the image.
			if (!glusImageLoadTga(materialWalker->material.diffuseTextureFilename, &image))
			{
				return GLUS_FALSE;
			}

			// Generate and bind a texture.
			glGenTextures(1, &materialWalker->material.diffuseTextureName);
			glBindTexture(GL_TEXTURE_2D, materialWalker->material.diffuseTextureName);

			// Transfer the image data from the CPU to the GPU.
			glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

			// Setting the texture parameters.
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

			glBindTexture(GL_TEXTURE_2D, 0);

			glusImageDestroyTga(&image);
		}

		materialWalker = materialWalker->next;
	}

	glActiveTexture(GL_TEXTURE0);

    //
    // Matrix setup.
    //

    glusMatrix4x4LookAtf(g_viewMatrix, 0.0f, 0.0f, EYE, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glUniformMatrix4fv(g_viewMatrixLocation, 1, GL_FALSE, g_viewMatrix);

    //

    glusMatrix4x4Orthof(g_projectionMatrix, -(GLfloat)VOXEL_GRID_SIZE * 0.5f, (GLfloat)VOXEL_GRID_SIZE * 0.5f, -(GLfloat)VOXEL_GRID_SIZE * 0.5f, (GLfloat)VOXEL_GRID_SIZE * 0.5f, NEAR, FAR);

    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, g_projectionMatrix);

    //

    // Calculate the half dimension of a pixel in NDC coordinates.
    // NDC coordinates go from -1.0 to 1.0. So (1.0 - (-1.0)) * 0.5 / length is half the length.
    halfPixelSize[0] = 1.0f / (GLfloat)VOXEL_GRID_SIZE;
    halfPixelSize[1] = 1.0f / (GLfloat)VOXEL_GRID_SIZE;

    glUniform2fv(g_halfPixelSizeLocation, 1, halfPixelSize);

    //

    glUseProgram(0);

    //
    // Voxel grid setup.
    //

	glGenTextures(1, &g_voxelGrid);
	glBindTexture(GL_TEXTURE_3D, g_voxelGrid);

	glTexImage3D(GL_TEXTURE_3D, 0, GL_R32UI, VOXEL_GRID_SIZE, VOXEL_GRID_SIZE, VOXEL_GRID_SIZE, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, 0);

	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_REPEAT);

	glBindTexture(GL_TEXTURE_3D, 0);

    //
    // Full screen rendering.
    //

    glusFileLoadText("../Example45/shader/fullscreen.vert.glsl", &vertexSource);
    glusFileLoadText("../Example45/shader/draw_voxels.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_fullscreenProgram, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    glUseProgram(g_fullscreenProgram.program);

    //

    glGenVertexArrays(1, &g_fullscreenVao);
    glBindVertexArray(g_fullscreenVao);

    glBindVertexArray(0);

    glUseProgram(0);

    //
    // Global OpenGL setup.
    //

    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);

    // Backface culling and depth test disabled by default.

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
	// Do nothing, as all initialization is done in the init() function.
}

GLUSboolean update(GLUSfloat time)
{
	// Reset color and counter value.
	static GLuint clearValue = {0x00000000};

	static GLfloat angle = 0.0f;

	GLUSgroupList* groupWalker;

	// Clear voxel grid.
	glClearTexImage(g_voxelGrid, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, &clearValue);

	// Clear background.
    glClear(GL_COLOR_BUFFER_BIT);

    //
    // Voxelize model.
    //

	// Disable color writing, as voxels are written into image.
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	glUseProgram(g_program.program);

	//
	// Rotate the model.
	//

    glusMatrix4x4Identityf(g_modelMatrix);

    glusMatrix4x4Translatef(g_modelMatrix, 0.0f, -200.0f * VOXEL_GRID_SIZE / WINDOW_SIZE, 0.0f);

    glusMatrix4x4RotateRyf(g_modelMatrix, angle);

    glusMatrix4x4Scalef(g_modelMatrix, 3000.0f * VOXEL_GRID_SIZE / WINDOW_SIZE, 3000.0f * VOXEL_GRID_SIZE / WINDOW_SIZE, 3000.0f * VOXEL_GRID_SIZE / WINDOW_SIZE);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, g_modelMatrix);

    angle += 10.0f * time;

	//

	glBindImageTexture(BINDING_VOXEL_GRID, g_voxelGrid, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);

    glViewport(0, 0, VOXEL_GRID_SIZE, VOXEL_GRID_SIZE);

	//
	// Render model.
	//

	glActiveTexture(GL_TEXTURE1);

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
		// Enable only texturing, if the material has a texture
		if (groupWalker->group.material->diffuseTextureName)
		{
			glBindTexture(GL_TEXTURE_2D, groupWalker->group.material->diffuseTextureName);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, 0);
		}

		glBindVertexArray(groupWalker->group.vao);

		glDrawElements(GL_TRIANGLES, groupWalker->group.numberIndices, GL_UNSIGNED_INT, 0);

		groupWalker = groupWalker->next;
	}

	glBindVertexArray(0);

	glActiveTexture(GL_TEXTURE0);

	//

    glBindImageTexture(BINDING_VOXEL_GRID, 0, 0, GL_TRUE, 0, GL_WRITE_ONLY, GL_R32UI);

    //
    // Dump the voxelized model to the screen.
    //

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

	glUseProgram(g_fullscreenProgram.program);

	glBindImageTexture(BINDING_VOXEL_GRID, g_voxelGrid, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

    glViewport(0, 0, WINDOW_SIZE, WINDOW_SIZE);

	glBindVertexArray(g_fullscreenVao);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	glBindImageTexture(BINDING_VOXEL_GRID, 0, 0, GL_TRUE, 0, GL_READ_ONLY, GL_R32UI);

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
	GLUSgroupList* groupWalker;
	GLUSmaterialList* materialWalker;

	glBindTexture(GL_TEXTURE_3D, 0);

	if (g_voxelGrid)
	{
		glDeleteTextures(1, &g_voxelGrid);

		g_voxelGrid = 0;
	}

	//

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (g_wavefront.verticesVBO)
	{
		glDeleteBuffers(1, &g_wavefront.verticesVBO);

		g_wavefront.verticesVBO = 0;
	}

	if (g_wavefront.texCoordsVBO)
	{
		glDeleteBuffers(1, &g_wavefront.texCoordsVBO);

		g_wavefront.texCoordsVBO = 0;
	}

	glBindVertexArray(0);

	groupWalker = g_wavefront.groups;
	while (groupWalker)
	{
	    if (groupWalker->group.indicesVBO)
	    {
	        glDeleteBuffers(1, &groupWalker->group.indicesVBO);

	        groupWalker->group.indicesVBO = 0;
	    }

		if (groupWalker->group.vao)
		{
			glDeleteVertexArrays(1, &groupWalker->group.vao);

			groupWalker->group.vao = 0;
		}

		groupWalker = groupWalker->next;
	}

	glBindTexture(GL_TEXTURE_2D, 0);

	materialWalker = g_wavefront.materials;
	while (materialWalker)
	{
		if (materialWalker->material.diffuseTextureName)
		{
			glDeleteTextures(1, &materialWalker->material.diffuseTextureName);

			materialWalker->material.diffuseTextureName = 0;
		}

		materialWalker = materialWalker->next;
	}

	//

    glUseProgram(0);

    glusProgramDestroy(&g_program);

    glusWavefrontDestroy(&g_wavefront);

    //

    if (g_fullscreenVao)
    {
        glDeleteVertexArrays(1, &g_fullscreenVao);

        g_fullscreenVao = 0;
    }

    glusProgramDestroy(&g_fullscreenProgram);
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
    		EGL_CONTEXT_MINOR_VERSION, 4,
    		EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
    		EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
			EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
    		EGL_NONE
    };

    glusWindowSetInitFunc(init);

    glusWindowSetReshapeFunc(reshape);

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

    if (!glusWindowCreate("GLUS Example Window", WINDOW_SIZE, WINDOW_SIZE, GLUS_FALSE, GLUS_TRUE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
