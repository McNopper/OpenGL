/**
 * OpenGL 4 - Example 23
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

static GLUSprogram g_program;

static GLint g_projectionMatrixLocation;

static GLint g_viewMatrixLocation;

static GLint g_modelMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_vertexLocation;

static GLint g_tangentLocation;

static GLint g_bitangentLocation;

static GLint g_normalLocation;

static GLint g_texCoordLocation;

static GLint g_colorTextureLocation;

static GLint g_normalTextureLocation;

static GLint g_displacementScaleLocation;

static GLint g_widthLocation;

static GLint g_heightLocation;

static GLint g_screenDistanceLocation;

static GLint g_doTessellateLocation;

//

static GLuint g_verticesVBO;

static GLuint g_tangentsVBO;

static GLuint g_bitangentsVBO;

static GLuint g_normalsVBO;

static GLuint g_texCoordsVBO;

static GLuint g_indicesVBO;

//

static GLuint g_vao;

//

static GLUSuint g_numberIndicesPlane;

//

static GLuint g_colorTexture;

static GLuint g_normalTexture;

//

static const GLfloat g_displacementScale = 0.3;

GLUSboolean init(GLUSvoid)
{
    GLfloat viewMatrix[16];

    GLUSshape plane;

	GLUStextfile vertexSource;
	GLUStextfile controlSource;
	GLUStextfile evaluationSource;
	GLUStextfile geometrySource;
	GLUStextfile fragmentSource;

	GLUStgaimage image;

	glusFileLoadText("../Example23/shader/tessellation.vert.glsl", &vertexSource);
	glusFileLoadText("../Example23/shader/tessellation.cont.glsl", &controlSource);
	glusFileLoadText("../Example23/shader/tessellation.eval.glsl", &evaluationSource);
	glusFileLoadText("../Example23/shader/tessellation.geom.glsl", &geometrySource);
	glusFileLoadText("../Example23/shader/tessellation.frag.glsl", &fragmentSource);

	glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, (const GLUSchar**) &controlSource.text, (const GLUSchar**) &evaluationSource.text, (const GLUSchar**) &geometrySource.text, (const GLUSchar**) &fragmentSource.text);

	glusFileDestroyText(&vertexSource);
	glusFileDestroyText(&controlSource);
	glusFileDestroyText(&evaluationSource);
	glusFileDestroyText(&geometrySource);
	glusFileDestroyText(&fragmentSource);

	//

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_viewMatrixLocation = glGetUniformLocation(g_program.program, "u_viewMatrix");
    g_modelMatrixLocation = glGetUniformLocation(g_program.program, "u_modelMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");

    g_colorTextureLocation = glGetUniformLocation(g_program.program, "u_colorTexture");
    g_normalTextureLocation = glGetUniformLocation(g_program.program, "u_normalTexture");

    g_displacementScaleLocation = glGetUniformLocation(g_program.program, "u_displacementScale");

    g_screenDistanceLocation = glGetUniformLocation(g_program.program, "u_screenDistance");

    g_doTessellateLocation = glGetUniformLocation(g_program.program, "u_doTessellate");

    g_widthLocation = glGetUniformLocation(g_program.program, "u_width");
    g_heightLocation = glGetUniformLocation(g_program.program, "u_height");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_tangentLocation = glGetAttribLocation(g_program.program, "a_tangent");
    g_bitangentLocation = glGetAttribLocation(g_program.program, "a_bitangent");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");
    g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");

    //

    // Load the image.
    glusImageLoadTga("four_shapes_color.tga", &image);

    glActiveTexture(GL_TEXTURE0);

    // Generate and bind a texture.
    glGenTextures(1, &g_colorTexture);
    glBindTexture(GL_TEXTURE_2D, g_colorTexture);

    // Transfer the image data from the CPU to the GPU.
    glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

    // Setting the texture parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glusImageDestroyTga(&image);

    //

    // Load the image.
    glusImageLoadTga("four_shapes_normal.tga", &image);

    glActiveTexture(GL_TEXTURE1);

    // Generate and bind a texture.
    glGenTextures(1, &g_normalTexture);
    glBindTexture(GL_TEXTURE_2D, g_normalTexture);

    // Transfer the image data from the CPU to the GPU.
    glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

    // Setting the texture parameters.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glusImageDestroyTga(&image);

    //

    glActiveTexture(GL_TEXTURE0);

	//

    // Core grid
    glusShapeCreateRectangularGridPlanef(&plane, 3.0f, 3.0f, 16, 16, GLUS_FALSE);

    g_numberIndicesPlane = plane.numberIndices;

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) plane.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_tangentsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_tangentsVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) plane.tangents, GL_STATIC_DRAW);

    glGenBuffers(1, &g_bitangentsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_bitangentsVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) plane.bitangents, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) plane.normals, GL_STATIC_DRAW);

    glGenBuffers(1, &g_texCoordsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 2 * sizeof(GLfloat), (GLfloat*) plane.texCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_numberIndicesPlane * sizeof(GLuint), (GLuint*) plane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&plane);

    //

    glUseProgram(g_program.program);

    // Calculate the view matrix ...
    glusMatrix4x4LookAtf(viewMatrix, 0.0f, 4.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glUniformMatrix4fv(g_viewMatrixLocation, 1, GL_FALSE, viewMatrix);

    // Global variable to scale the displacement
    glUniform1f(g_displacementScaleLocation, g_displacementScale);

    // Tessellate smaller than the given distance
    glUniform1f(g_screenDistanceLocation, 8.0f);

    // Enable tesselllation
    glUniform1i(g_doTessellateLocation, 1);

    //

    glUniform1i(g_colorTextureLocation, 0);

    glUniform1i(g_normalTextureLocation, 1);

    //

	glGenVertexArrays(1, &g_vao);
	glBindVertexArray(g_vao);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
	glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_vertexLocation);

	glBindBuffer(GL_ARRAY_BUFFER, g_tangentsVBO);
	glVertexAttribPointer(g_tangentLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_tangentLocation);

	glBindBuffer(GL_ARRAY_BUFFER, g_bitangentsVBO);
	glVertexAttribPointer(g_bitangentLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_bitangentLocation);

	glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
	glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_normalLocation);

	glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
	glVertexAttribPointer(g_texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_texCoordLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

	//

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glEnable(GL_DEPTH_TEST);

	glEnable(GL_CULL_FACE);

	// We process triangle patches
	glPatchParameteri(GL_PATCH_VERTICES, 3);

	return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    GLfloat projectionMatrix[16];

	glViewport(0, 0, width, height);

	// Needed for screen space dependent tessellation
	glUniform1f(g_widthLocation, (GLfloat) width);
	glUniform1f(g_heightLocation, (GLfloat) height);

	glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 0.1f, 100.0f);

	glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSvoid key(GLUSboolean pressed, GLUSint key)
{
	static GLboolean displacement = GL_TRUE;

	static GLboolean tessellation = GL_TRUE;

	static GLboolean wireframe = GL_FALSE;

	if (pressed && key == 'd')
	{
		displacement = !displacement;

		if (displacement)
		{
			glUniform1f(g_displacementScaleLocation, g_displacementScale);
		}
		else
		{
			glUniform1f(g_displacementScaleLocation, 0.0f);
		}
	}

	if (pressed && key == 't')
	{
		tessellation = !tessellation;

		if (tessellation)
		{
			glUniform1i(g_doTessellateLocation, 1);
		}
		else
		{
			glUniform1i(g_doTessellateLocation, 0);
		}
	}


	if (pressed && key == 'w')
	{
		wireframe = !wireframe;

		if (wireframe)
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		}
		else
		{
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
	}
}

GLUSboolean update(GLUSfloat time)
{
	static GLfloat angle = 0.0f;

    GLfloat modelMatrix[16];
    GLfloat normalMatrix[9];

    // Calculate the model matrix ...
    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4RotateRzRyRxf(modelMatrix, 0.0f, angle, -90.0f);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);

    // Calculate the normal matrix ...
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    //

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glDrawElements(GL_PATCHES, g_numberIndicesPlane, GL_UNSIGNED_INT, 0);

	//

	angle += time * 45.0f;

	return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glBindTexture(GL_TEXTURE_2D, 0);

    if (g_colorTexture)
    {
        glDeleteTextures(1, &g_colorTexture);

        g_colorTexture = 0;
    }

    if (g_normalTexture)
    {
        glDeleteTextures(1, &g_normalTexture);

        g_normalTexture = 0;
    }

    //

    glBindBuffer(GL_ARRAY_BUFFER, 0);

	if (g_verticesVBO)
	{
		glDeleteBuffers(1, &g_verticesVBO);

		g_verticesVBO = 0;
	}

	if (g_tangentsVBO)
	{
		glDeleteBuffers(1, &g_tangentsVBO);

		g_tangentsVBO = 0;
	}

	if (g_bitangentsVBO)
	{
		glDeleteBuffers(1, &g_bitangentsVBO);

		g_bitangentsVBO = 0;
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

    glBindVertexArray(0);

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_vao);

        g_vao = 0;
    }

	glUseProgram(0);

	glusProgramDestroy(&g_program);
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

    if (!glusWindowCreate("GLUS Example Window", 640, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    // Print out the keys
    printf("Keys:\n");
    printf("d = Toggle displacement on/off\n");
    printf("t = Toggle tessellation on/off\n");
    printf("w = Toggle wireframe on/off\n");

    glusWindowRun();

    return 0;
}
