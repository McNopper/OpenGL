/**
 * OpenGL 3 - Example 05
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <stdio.h>

#include "GL/glus.h"

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
};

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

/**
 * The locations for the light properties.
 */
static struct LightLocations g_light;

/**
 * The locations for the material properties.
 */
static struct MaterialLocations g_material;

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_indicesVBO;

static GLuint g_vao;

static GLuint g_numberIndicesSphere;

GLUSboolean init(GLUSvoid)
{
    GLfloat viewMatrix[16];
    GLfloat normalMatrix[9];

    // This is a white light.
    struct LightProperties light = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

    // Blue color material with white specular color.
    struct MaterialProperties material = { { 0.0f, 0.0f, 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, 20.0f };

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape sphere;

    glusFileLoadText("../Example05/shader/phong.vert.glsl", &vertexSource);
    glusFileLoadText("../Example05/shader/phong.frag.glsl", &fragmentSource);

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

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    // Use a helper function to create a sphere.
    glusShapeCreateSpheref(&sphere, 0.5f, 32);

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

    //

    glusMatrix4x4LookAtf(viewMatrix, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    // The calculations are done in camera / view space. So pass the view matrix, which is a rigid body transform.
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, viewMatrix);

    glUniformMatrix4fv(g_modelViewMatrixLocation, 1, GL_FALSE, viewMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    //

    glusVector3Normalizef(light.direction);

    // Transform light to camera space, as it is currently in world space.
    glusMatrix4x4MultiplyVector3f(light.direction, viewMatrix, light.direction);

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

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

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
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, g_numberIndicesSphere, GL_UNSIGNED_INT, 0);

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

    if (!glusWindowCreate("GLUS Example Window", 640, 480, GLUS_FALSE, GLUS_FALSE, eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
