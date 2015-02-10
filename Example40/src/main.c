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

#include "render_sphere.h"

#define BINDING_BUFFER_COMP_VERTICES_IN_PREVIOUS 1
#define BINDING_BUFFER_COMP_VERTICES_IN_CURRENT 2
#define BINDING_BUFFER_COMP_VERTICES_OUT 3
#define BINDING_BUFFER_COMP_NORMALS_OUT 4

// Row is between two vertices, so 31 rows generates 32 vertices per side.
#define ROWS	31

static GLUSprogram g_computeProgram;

static GLint g_verticesPerRowLocation;

static GLint g_deltaTimeLocation;

static GLint g_distanceRestLocation;
static GLint g_distanceDiagonalRestLocation;

static GLint g_sphereCenterLocation;
static GLint g_sphereRadiusLocation;

//

static GLUSprogram g_program;

static GLint g_modelViewProjectionMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_colorLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLuint g_indicesVBO;

static GLuint g_vao;

static GLuint g_numberIndicesPlane;

//

static GLuint g_verticesBuffer[3];

static GLuint g_normalsBuffer;

//

static GLUSshape g_gridPlane;

GLUSboolean init(GLUSvoid)
{
    GLfloat lightDirection[3] = { 1.0f, 1.0f, 1.0f };
    GLfloat color[4] = { 0.0f, 1.0f, 1.0f, 1.0f };

    GLUStextfile computeSource;

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLint i;
    GLfloat matrix[16];

    GLfloat* normals;

    GLfloat distanceRest;
    GLfloat distanceDiagonalRest;

    GLfloat sphereCenter[4] = {0.0f, 0.0f, -0.01f, 1.0f};
    GLfloat sphereRadius = 1.0f;

    glusFileLoadText("../Example40/shader/cloth.comp.glsl", &computeSource);

    glusProgramBuildComputeFromSource(&g_computeProgram, (const GLchar**) &computeSource.text);

    glusFileDestroyText(&computeSource);


    glusFileLoadText("../Example40/shader/cloth.vert.glsl", &vertexSource);
    glusFileLoadText("../Example40/shader/cloth.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_verticesPerRowLocation = glGetUniformLocation(g_computeProgram.program, "u_verticesPerRow");
    g_deltaTimeLocation = glGetUniformLocation(g_computeProgram.program, "u_deltaTime");
    g_distanceRestLocation = glGetUniformLocation(g_computeProgram.program, "u_distanceRest");
    g_distanceDiagonalRestLocation = glGetUniformLocation(g_computeProgram.program, "u_distanceDiagonalRest");
    g_sphereCenterLocation = glGetUniformLocation(g_computeProgram.program, "u_sphereCenter");
    g_sphereRadiusLocation = glGetUniformLocation(g_computeProgram.program, "u_sphereRadius");


    g_modelViewProjectionMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewProjectionMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");
    g_colorLocation = glGetUniformLocation(g_program.program, "u_color");

	g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
	g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    // Use a helper function to create a grid plane.
    glusShapeCreateRectangularGridPlanef(&g_gridPlane, 2.0f, 2.0f, ROWS, ROWS, GLUS_FALSE);

    // Use x, as only horizontal and vertical springs are used. Adapt this, if diagonal or a non square grid is used.
    distanceRest = g_gridPlane.vertices[4] - g_gridPlane.vertices[0];
    distanceDiagonalRest = sqrtf(2.0f * distanceRest * distanceRest);

    // Rotate by 90 degrees, that the grid is in the x-z-plane.
    glusMatrix4x4Identityf(matrix);
    glusMatrix4x4Translatef(matrix, 0.0f, 1.1f, 0.0f);
    glusMatrix4x4RotateRxf(matrix, -90.0f);
    for (i = 0; i < g_gridPlane.numberVertices; i++)
    {
    	glusMatrix4x4MultiplyPoint4f(&g_gridPlane.vertices[4 * i], matrix, &g_gridPlane.vertices[4 * i]);
    }

    g_numberIndicesPlane = g_gridPlane.numberIndices;

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, g_gridPlane.numberIndices * sizeof(GLuint), (GLuint*) g_gridPlane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    //

    normals = (GLfloat*)malloc(g_gridPlane.numberVertices * 4 * sizeof(GLfloat));

    // Add one more GLfloat channel as padding for std430 layout.
    glusPaddingConvertf(normals, g_gridPlane.normals, 3, 1, g_gridPlane.numberVertices);

    free(g_gridPlane.normals);
    g_gridPlane.normals = normals;

    //

	glGenBuffers(3, g_verticesBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_verticesBuffer[0]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), g_gridPlane.vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_verticesBuffer[1]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), g_gridPlane.vertices, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_verticesBuffer[2]);
	glBufferData(GL_SHADER_STORAGE_BUFFER, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), 0, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &g_normalsBuffer);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_normalsBuffer);
	glBufferData(GL_SHADER_STORAGE_BUFFER, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), g_gridPlane.normals, GL_DYNAMIC_DRAW);

    //

    glUseProgram(g_program.program);

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

	glBindBuffer(GL_ARRAY_BUFFER, g_normalsBuffer);
	glVertexAttribPointer(g_normalLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_normalLocation);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesBuffer[2]);
	glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

    glBindVertexArray(0);

    //

    glusVector3Normalizef(lightDirection);
    glUniform3fv(g_lightDirectionLocation, 1, lightDirection);

    glUniform4fv(g_colorLocation, 1, color);

    glUseProgram(0);

    //

    glUseProgram(g_computeProgram.program);

    glUniform1i(g_verticesPerRowLocation, ROWS + 1);

    glUniform1f(g_distanceRestLocation, distanceRest);
    glUniform1f(g_distanceDiagonalRestLocation, distanceDiagonalRest);

    glUniform4fv(g_sphereCenterLocation, 1, sphereCenter);
    glUniform1f(g_sphereRadiusLocation, sphereRadius);

    glUseProgram(0);

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    //

    if (!initSphere(sphereCenter, sphereRadius, lightDirection))
    {
    	return GLUS_FALSE;
    }

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    GLfloat modelMatrix[16];
    GLfloat normalMatrix[9];
    GLfloat viewMatrix[16];
    GLfloat projectionMatrix[16];
    GLfloat viewProjectionMatrix[16];
    GLfloat modelViewProjectionMatrix[16];

    glUseProgram(g_program.program);

    glViewport(0, 0, width, height);

    // For now, the grid is just on the "ground".
    glusMatrix4x4Identityf(modelMatrix);

    // This model matrix is a rigid body transform. So no need for the inverse, transposed matrix.
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelMatrix);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    glusMatrix4x4LookAtf(viewMatrix, 0.0f, 4.0f, 4.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glusMatrix4x4Multiplyf(viewProjectionMatrix, projectionMatrix, viewMatrix);
    glusMatrix4x4Multiplyf(modelViewProjectionMatrix, viewProjectionMatrix, modelMatrix);

    glUniformMatrix4fv(g_modelViewProjectionMatrixLocation, 1, GL_FALSE, modelViewProjectionMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    glUseProgram(0);

    reshapeSphere(viewProjectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
	static GLfloat totalTime = 0.0f;

	static GLint previousInput = 0;
	static GLint currentInput = 1;
	static GLint currentOutput = 2;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);

    if (!updateSphere(time))
    {
    	return GLUS_FALSE;
    }

    glDisable(GL_CULL_FACE);

    //
    // Simulation part.
    //

    glUseProgram(g_computeProgram.program);

    glUniform1f(g_deltaTimeLocation, time);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_VERTICES_IN_PREVIOUS, g_verticesBuffer[previousInput]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_VERTICES_IN_CURRENT, g_verticesBuffer[currentInput]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_VERTICES_OUT, g_verticesBuffer[currentOutput]);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_NORMALS_OUT, g_normalsBuffer);

    // Process all vertices.
    glDispatchCompute(1, 1, 1);

    // Make sure, all vertices and normals are written.
    glMemoryBarrier(GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_VERTICES_IN_PREVIOUS, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_VERTICES_IN_CURRENT, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_VERTICES_OUT, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, BINDING_BUFFER_COMP_NORMALS_OUT, 0);

    //
    // Drawing part.
    //

    glUseProgram(g_program.program);

    glBindVertexArray(g_vao);

	glBindBuffer(GL_ARRAY_BUFFER, g_verticesBuffer[currentOutput]);
	glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawElements(GL_TRIANGLES, g_numberIndicesPlane, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    // Output is next time input etc.

    previousInput = (previousInput + 1) % 3;
    currentInput = (currentInput + 1) % 3;
    currentOutput = (currentOutput + 1) % 3;

    // Update the total passed time.

    totalTime += time;

    // Reset after 10 seconds.
    if (totalTime >= 10.0f)
    {
        previousInput = 0;
        currentInput = 1;
        currentOutput = 2;

    	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_verticesBuffer[previousInput]);
    	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), g_gridPlane.vertices);

    	glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_verticesBuffer[currentInput]);
    	glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, g_gridPlane.numberVertices * 4 * sizeof(GLfloat), g_gridPlane.vertices);

    	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

    	totalTime = 0.0f;
    }

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
	terminateSphere();

	//

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

	if (g_verticesBuffer[2])
	{
		glDeleteBuffers(1, &g_verticesBuffer[2]);

		g_verticesBuffer[2] = 0;
	}

	if (g_normalsBuffer)
	{
		glDeleteBuffers(1, &g_normalsBuffer);

		g_normalsBuffer = 0;
	}

    //

    glUseProgram(0);

    glusProgramDestroy(&g_computeProgram);

    glusProgramDestroy(&g_program);

    //

    glusShapeDestroyf(&g_gridPlane);
}

int main(int argc, char* argv[])
{
	EGLint eglConfigAttributes[] = {
	        EGL_RED_SIZE, 8,
	        EGL_GREEN_SIZE, 8,
	        EGL_BLUE_SIZE, 8,
	        EGL_DEPTH_SIZE, 24,
	        EGL_STENCIL_SIZE, 0,
	        EGL_SAMPLE_BUFFERS, 1,
	        EGL_SAMPLES, 4,
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

    glusWindowSetUpdateFunc(update);

    glusWindowSetTerminateFunc(terminate);

    if (!glusWindowCreate("GLUS Example Window", 1024, 768, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
