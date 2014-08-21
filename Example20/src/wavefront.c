/**
 * OpenGL 3 - Example 20 - Rendering of the wavefront object
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

static GLfloat* g_viewMatrix;

static GLUSprogram g_program;

/**
 * The location of the projection matrix.
 */
static GLint g_projectionMatrixLocation;

/**
 * The location of the model view matrix.
 */
static GLint g_modelViewMatrixLocation;

static GLint g_normalMatrixLocation;

static GLint g_vertexLocation;

static GLint g_normalLocation;

/**
 * The locations for the light properties.
 */
static struct LightLocations g_lightLocations;

/**
 * The locations for the material properties.
 */
static struct MaterialLocations g_material;


static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_vao;

static GLuint g_numberVertices;

// Global white light.
static struct LightProperties* g_light;

GLUSboolean initWavefront(GLUSfloat viewMatrix[16], struct LightProperties* light)
{
    // Color material with white specular color.
    struct MaterialProperties material = { { 1.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 20.0f };

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape wavefrontObj;

    // Use the same view matrix, so keep a reference.
    g_viewMatrix = viewMatrix;

    g_light = light;

    //

    glusFileLoadText("../Example20/shader/phong.vert.glsl", &vertexSource);
    glusFileLoadText("../Example20/shader/phong.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_modelViewMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");

    g_lightLocations.directionLocation = glGetUniformLocation(g_program.program, "u_light.direction");
    g_lightLocations.ambientColorLocation = glGetUniformLocation(g_program.program, "u_light.ambientColor");
    g_lightLocations.diffuseColorLocation = glGetUniformLocation(g_program.program, "u_light.diffuseColor");
    g_lightLocations.specularColorLocation = glGetUniformLocation(g_program.program, "u_light.specularColor");

    g_material.ambientColorLocation = glGetUniformLocation(g_program.program, "u_material.ambientColor");
    g_material.diffuseColorLocation = glGetUniformLocation(g_program.program, "u_material.diffuseColor");
    g_material.specularColorLocation = glGetUniformLocation(g_program.program, "u_material.specularColor");
    g_material.specularExponentLocation = glGetUniformLocation(g_program.program, "u_material.specularExponent");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    // Use a helper function to load an wavefront object file.
    glusShapeLoadWavefront("elephant.obj", &wavefrontObj);

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

    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_normalLocation);

    //

    glusVector3Normalizef(g_light->direction);

    // Set up light ...
    glUniform3fv(g_lightLocations.directionLocation, 1, g_light->direction);
    glUniform4fv(g_lightLocations.ambientColorLocation, 1, g_light->ambientColor);
    glUniform4fv(g_lightLocations.diffuseColorLocation, 1, g_light->diffuseColor);
    glUniform4fv(g_lightLocations.specularColorLocation, 1, g_light->specularColor);

    // ... and material values.
    glUniform4fv(g_material.ambientColorLocation, 1, material.ambientColor);
    glUniform4fv(g_material.diffuseColorLocation, 1, material.diffuseColor);
    glUniform4fv(g_material.specularColorLocation, 1, material.specularColor);
    glUniform1f(g_material.specularExponentLocation, material.specularExponent);

    return GLUS_TRUE;
}

GLUSvoid reshapeWavefront(GLUSfloat projectionMatrix[16])
{
    glUseProgram(g_program.program);

    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean updateWavefront(GLUSfloat time)
{
    static GLfloat angle = 0.0f;

    GLfloat modelViewMatrix[16];
    GLfloat normalMatrix[9];

    glusMatrix4x4Identityf(modelViewMatrix);
    glusMatrix4x4Translatef(modelViewMatrix, 0.0f, -1.5f, 0.0f);
    glusMatrix4x4RotateRyf(modelViewMatrix, angle);
    glusMatrix4x4Multiplyf(modelViewMatrix, g_viewMatrix, modelViewMatrix);

    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUseProgram(g_program.program);

    glUniformMatrix4fv(g_modelViewMatrixLocation, 1, GL_FALSE, modelViewMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    glBindVertexArray(g_vao);

    glDrawArrays(GL_TRIANGLES, 0, g_numberVertices);

    angle += 30.0f * time;

    return GLUS_TRUE;
}

GLUSvoid terminateWavefront(GLUSvoid)
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

    glUseProgram(0);

    glusProgramDestroy(&g_program);
}
