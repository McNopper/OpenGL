/**
 * OpenGL 4 - Example 40
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

/**
 * OpenGL 3 - Example 04
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include "render_sphere.h"

static GLUSprogram g_program;

static GLint g_modelViewProjectionMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_lightDirectionLocation;

static GLint g_colorLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_indicesVBO;

static GLuint g_vao;

static GLuint g_numberIndicesSphere;

//

static GLfloat g_sphereCenter[4];

GLUSboolean initSphere(GLUSfloat sphereCenter[4], GLUSfloat sphereRadius, GLUSfloat lightDirection[3])
{
    GLfloat color[4] = { 1.0f, 1.0f, 0.0f, 1.0f };

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape sphere;

    glusFileLoadText("../Example40/shader/sphere.vert.glsl", &vertexSource);
    glusFileLoadText("../Example40/shader/sphere.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    // Retrieve the uniform locations in the program.
    g_modelViewProjectionMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewProjectionMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");
    g_colorLocation = glGetUniformLocation(g_program.program, "u_color");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    glusShapeCreateSpheref(&sphere, sphereRadius, 64);

    g_numberIndicesSphere = sphere.numberIndices;

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) sphere.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) sphere.normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.numberIndices * sizeof(GLuint), (GLuint*) sphere.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&sphere);

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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesVBO);

    glBindVertexArray(0);

    //

    // Set the light direction ...
    glUniform3fv(g_lightDirectionLocation, 1, lightDirection);

    // ... and the yellow color for the sphere.
    glUniform4fv(g_colorLocation, 1, color);

    glUseProgram(0);

    //

    g_sphereCenter[0] = sphereCenter[0];
    g_sphereCenter[1] = sphereCenter[1];
    g_sphereCenter[2] = sphereCenter[2];
    g_sphereCenter[3] = sphereCenter[3];

    return GLUS_TRUE;
}

GLUSvoid reshapeSphere(GLUSfloat viewProjectionMatrix[16])
{
    GLfloat modelMatrix[16];
    GLfloat normalMatrix[9];
    GLfloat modelViewProjectionMatrix[16];

    glUseProgram(g_program.program);

    // Initialize with the identity matrix ...
    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4Translatef(modelMatrix, g_sphereCenter[0], g_sphereCenter[1], g_sphereCenter[2]);

    // This model matrix is a rigid body transform. So no need for the inverse, transposed matrix.
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelMatrix);

    glusMatrix4x4Multiplyf(modelViewProjectionMatrix, viewProjectionMatrix, modelMatrix);

    glUniformMatrix4fv(g_modelViewProjectionMatrixLocation, 1, GL_FALSE, modelViewProjectionMatrix);

    // Set the normal matrix.
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    glUseProgram(0);
}

GLUSboolean updateSphere(GLUSfloat time)
{
	glUseProgram(g_program.program);

	glBindVertexArray(g_vao);

    glDrawElements(GL_TRIANGLES, g_numberIndicesSphere, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);

    glUseProgram(0);

    return GLUS_TRUE;
}

GLUSvoid terminateSphere(GLUSvoid)
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
