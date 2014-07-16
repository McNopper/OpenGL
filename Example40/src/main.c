/**
 * OpenGL 4 - Example 40
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

#define BINDING_BUFFER_COMP_VERTICES_IN 1
#define BINDING_BUFFER_COMP_VERTICES_OUT 2
#define BINDING_BUFFER_COMP_NORMALS_OUT 3

#define BINDING_BUFFER_VERT_VERTICES 1
#define BINDING_BUFFER_VERT_NORMALS 2

// If these values change, also change in compute shader.
#define ROWS	10
#define COLUMNS 10

static GLUSshaderprogram g_computeProgram;

//

static GLUSshaderprogram g_program;

static GLint g_modelViewProjectionMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_colorLocation;

static GLuint g_indicesVBO;

static GLuint g_vao;

static GLuint g_numberIndicesPlane;

//

static GLuint g_verticesBuffer[2];

static GLuint g_normalsBuffer;

GLUSboolean init(GLUSvoid)
{
    GLfloat lightDirection[3] = { 1.0f, 1.0f, 1.0f };
    GLfloat color[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

    GLUStextfile computeSource;

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape gridPlane;

    GLint i;
    GLfloat matrix[16];

    GLfloat* normals;

    glusLoadTextFile("../Example40/shader/cloth.comp.glsl", &computeSource);

    glusBuildComputeProgramFromSource(&g_computeProgram, (const GLchar**) &computeSource.text);

    glusDestroyTextFile(&computeSource);


    glusLoadTextFile("../Example40/shader/cloth.vert.glsl", &vertexSource);
    glusLoadTextFile("../Example40/shader/cloth.frag.glsl", &fragmentSource);

    glusBuildProgramFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusDestroyTextFile(&vertexSource);
    glusDestroyTextFile(&fragmentSource);

    //

    g_modelViewProjectionMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewProjectionMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");
    g_colorLocation = glGetUniformLocation(g_program.program, "u_color");

    //

    // Use a helper function to create a grid plane.
    glusCreateRectangularGridPlanef(&gridPlane, 5.0f, 5.0f, 10, 10, GLUS_TRUE);

    // Rotate by 90 degrees, that the grid is in the x-z-plane.
    glusMatrix4x4Identityf(matrix);
    glusMatrix4x4RotateRxf(matrix, -90.0f);
    for (i = 0; i < gridPlane.numberVertices; i++)
    {
    	glusMatrix4x4MultiplyPoint4f(&gridPlane.vertices[4 * i], matrix, &gridPlane.vertices[4 * i]);
    }

    g_numberIndicesPlane = gridPlane.numberIndices;

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridPlane.numberIndices * sizeof(GLuint), (GLuint*) gridPlane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //

    normals = (GLfloat*)malloc(gridPlane.numberVertices * 4 * sizeof(GLfloat));

    // Add one more GLfloat channel as padding for std430 layout.
    glusPaddingConvertf(normals, gridPlane.normals, 3, 1, gridPlane.numberVertices);

	glGenBuffers(2, g_verticesBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_verticesBuffer[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, gridPlane.numberVertices * 4 * sizeof(GLfloat), gridPlane.vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_verticesBuffer[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, gridPlane.numberVertices * 4 * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &g_normalsBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_normalsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, gridPlane.numberVertices * 4 * sizeof(GLfloat), normals, GL_DYNAMIC_DRAW);

	free(normals);

    //

    glusDestroyShapef(&gridPlane);

    //

    glUseProgram(g_program.program);

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

    //

    glusVector3Normalizef(lightDirection);
    glUniform3fv(g_lightDirectionLocation, 1, lightDirection);

    glUniform4fv(g_colorLocation, 1, color);

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
    GLfloat normalMatrix[9];
    GLfloat viewMatrix[16];
    GLfloat modelViewProjectionMatrix[16];

    glViewport(0, 0, width, height);

    // For now, the grid is just on the "ground".
    glusMatrix4x4Identityf(modelMatrix);

    // This model matrix is a rigid body transform. So no need for the inverse, transposed matrix.
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelMatrix);

    glusPerspectivef(modelViewProjectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    glusLookAtf(viewMatrix, 0.0f, 2.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glusMatrix4x4Multiplyf(modelViewProjectionMatrix, modelViewProjectionMatrix, viewMatrix);
    glusMatrix4x4Multiplyf(modelViewProjectionMatrix, modelViewProjectionMatrix, modelMatrix);

    glUniformMatrix4fv(g_modelViewProjectionMatrixLocation, 1, GL_FALSE, modelViewProjectionMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);
}

GLUSboolean update(GLUSfloat time)
{
	static GLint currentInput = 0;
	static GLint currentOutput = 1;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //
    // Simulation part.
    //

    glUseProgram(g_computeProgram.program);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_VERTICES_IN, g_verticesBuffer[currentInput]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_VERTICES_OUT, g_verticesBuffer[currentOutput]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_NORMALS_OUT, g_normalsBuffer);

    // Process all vertices.
    glDispatchCompute((ROWS + 1) * (COLUMNS + 1), 1, 1);

    // Make sure, all vertices and normals are written.
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_VERTICES_IN, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_VERTICES_OUT, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_NORMALS_OUT, 0);

    //
    // Drawing part.
    //

    glUseProgram(g_program.program);

    glBindVertexArray(g_vao);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_VERT_VERTICES, g_verticesBuffer[currentOutput]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_VERT_NORMALS, g_normalsBuffer);

    glDrawElements(GL_TRIANGLE_STRIP, g_numberIndicesPlane, GL_UNSIGNED_INT, 0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_VERT_VERTICES, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_VERT_NORMALS, 0);

    glBindVertexArray(0);

    // Output is next time input etc.

    currentInput = (currentInput + 1) % 2;
    currentInput = (currentOutput + 1) % 2;

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
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

    //

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	if (g_verticesBuffer[0])
	{
		glDeleteBuffers(1, &g_verticesBuffer[0]);

		g_verticesBuffer[0] = 0;
	}

	if (g_verticesBuffer[1])
	{
		glDeleteBuffers(1, &g_verticesBuffer[1]);

		g_verticesBuffer[1] = 0;
	}

	if (g_normalsBuffer)
	{
		glDeleteBuffers(1, &g_normalsBuffer);

		g_normalsBuffer = 0;
	}

    //

    glUseProgram(0);

    glusDestroyProgram(&g_computeProgram);

    glusDestroyProgram(&g_program);
}

int main(int argc, char* argv[])
{
    glusInitFunc(init);

    glusReshapeFunc(reshape);

    glusUpdateFunc(update);

    glusTerminateFunc(terminate);

    glusPrepareContext(4, 3, GLUS_FORWARD_COMPATIBLE_BIT);

    glusPrepareMSAA(4);

    glusPrepareDebug(GLUS_TRUE);

    if (!glusCreateWindow("GLUS Example Window", 1024, 768, 24, 0, GLUS_FALSE))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusRun();

    return 0;
}
