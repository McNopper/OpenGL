/**
 * OpenGL 3 - Example 18
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

static GLfloat g_viewMatrix[16];

/**
 * The location of the projection matrix.
 */
static GLint g_projectionMatrixLocation;

/**
 * The location of the view matrix.
 */
static GLint g_viewMatrixLocation;

/**
 * The location of the model matrix.
 */
static GLint g_modelMatrixLocation;

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

static GLint g_planeLocation;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_indicesVBO;

static GLuint g_vao;

static GLuint g_numberIndicesSphere;

//

static GLuint g_verticesPlaneVBO;

static GLuint g_normalsPlaneVBO;

static GLuint g_indicesPlaneVBO;

static GLuint g_planeVAO;

static GLuint g_numberIndicesPlane;

GLUSboolean init(GLUSvoid)
{
    // This is a white light.
    struct LightProperties light = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

    // Blue color material with white specular color.
    struct MaterialProperties material = { { 0.0f, 0.1f, 0.1f, 1.0f }, { 0.0f, 0.8f, 0.8f, 1.0f }, { 0.6f, 0.6f, 0.6f, 1.0f }, 60.0f };

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUSshape sphere;

    GLUSshape plane;

    glusFileLoadText("../Example18/shader/phong.vert.glsl", &vertexSource);
    glusFileLoadText("../Example18/shader/phong.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_viewMatrixLocation = glGetUniformLocation(g_program.program, "u_viewMatrix");
    g_modelMatrixLocation = glGetUniformLocation(g_program.program, "u_modelMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");

    g_light.directionLocation = glGetUniformLocation(g_program.program, "u_light.direction");
    g_light.ambientColorLocation = glGetUniformLocation(g_program.program, "u_light.ambientColor");
    g_light.diffuseColorLocation = glGetUniformLocation(g_program.program, "u_light.diffuseColor");
    g_light.specularColorLocation = glGetUniformLocation(g_program.program, "u_light.specularColor");

    g_material.ambientColorLocation = glGetUniformLocation(g_program.program, "u_material.ambientColor");
    g_material.diffuseColorLocation = glGetUniformLocation(g_program.program, "u_material.diffuseColor");
    g_material.specularColorLocation = glGetUniformLocation(g_program.program, "u_material.specularColor");
    g_material.specularExponentLocation = glGetUniformLocation(g_program.program, "u_material.specularExponent");

    g_planeLocation = glGetUniformLocation(g_program.program, "u_plane");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    //

    // Use a helper function to create a sphere.
    glusShapeCreateSpheref(&sphere, 0.5f, 64);

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

    glusShapeCreatePlanef(&plane, 0.5f);

    g_numberIndicesPlane = plane.numberIndices;

    glGenBuffers(1, &g_verticesPlaneVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesPlaneVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) plane.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsPlaneVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsPlaneVBO);
    glBufferData(GL_ARRAY_BUFFER, plane.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) plane.normals, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glGenBuffers(1, &g_indicesPlaneVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesPlaneVBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, plane.numberIndices * sizeof(GLuint), (GLuint*) plane.indices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&plane);


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

    glGenVertexArrays(1, &g_planeVAO);
    glBindVertexArray(g_planeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesPlaneVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsPlaneVBO);
    glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_normalLocation);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_indicesPlaneVBO);

    //

    glusMatrix4x4LookAtf(g_viewMatrix, 0.0f, 0.0f, 3.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glUniformMatrix4fv(g_viewMatrixLocation, 1, GL_FALSE, g_viewMatrix);

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

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    // No back face culling, as we need it for the stencil algorithm.

    // Enable the stencil test - mainly needed for the plane.
    glEnable(GL_STENCIL_TEST);

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

	GLfloat modelMatrix[16];
	GLfloat modelViewMatrix[16];
	GLfloat normalMatrix[9];

	GLfloat planeMatrix[16];

	GLfloat clippingPlane[4] = {0.0f, 0.0f, -1.0f, 0.0f};

	GLfloat planeBecauseOfBug[4] = {0.0f, -1.0f, 0.0f, 100.0f};

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // Clipping plane

    glusMatrix4x4Identityf(planeMatrix);
    glusMatrix4x4RotateRyf(planeMatrix, angle);
    glusMatrix4x4Translatef(planeMatrix, 0.0f, 0.25, 0.25f);
    glusMatrix4x4RotateRxf(planeMatrix, -30.0f);

    // For planes, we do need the inverse, transposed matrix.
    glusMatrix4x4InverseRigidBodyf(planeMatrix);
    glusMatrix4x4Transposef(planeMatrix);

    glusMatrix4x4MultiplyPlanef(clippingPlane, planeMatrix, clippingPlane);

    glUniform4fv(g_planeLocation, 1, clippingPlane);

    // Sphere

    glusMatrix4x4Identityf(modelMatrix);
    glusMatrix4x4RotateRyf(modelMatrix, angle);

    glusMatrix4x4Multiplyf(modelViewMatrix, g_viewMatrix, modelMatrix);
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    glBindVertexArray(g_vao);

    // Always pass. The 0s are not used.
    glStencilFunc(GL_ALWAYS, 0, 0);
    // Visible elements gets twice inverted, because of the front and back face. So the value is 0.
    // The clipped area only gets inverted by the back face. The front face is clipped, so no second invert is done. The final value is 255 (assume an 8bit stencil buffer).
    glStencilOp(GL_KEEP, GL_INVERT, GL_INVERT);

    glEnable(GL_CLIP_DISTANCE0);

    glDrawElements(GL_TRIANGLES, g_numberIndicesSphere, GL_UNSIGNED_INT, 0);

    // Plane

    // Use the model matrix from the sphere and add the plane transforms.
    glusMatrix4x4Translatef(modelMatrix, 0.0f, 0.25, 0.25f);
    glusMatrix4x4RotateRxf(modelMatrix, -30.0f);

    glusMatrix4x4Multiplyf(modelViewMatrix, g_viewMatrix, modelMatrix);
    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    glUniformMatrix4fv(g_modelMatrixLocation, 1, GL_FALSE, modelMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    glBindVertexArray(g_planeVAO);

    // Render only, where 255 & 1 = 1 is set.
    glStencilFunc(GL_EQUAL, 1, 1);

    glDisable(GL_CLIP_DISTANCE0);
    // Note: Bug in the AMD driver. The above disable is not working: "Values written into gl_ClipDistance for planes that are not enabled have no effect."
    //       For a workaround, I am setting a far plane, which does not clip anything.
    glUniform4fv(g_planeLocation, 1, planeBecauseOfBug);

    glDrawElements(GL_TRIANGLES, g_numberIndicesPlane, GL_UNSIGNED_INT, 0);

    angle += 45.0f * time;

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

    //

    if (g_verticesPlaneVBO)
    {
        glDeleteBuffers(1, &g_verticesPlaneVBO);

        g_verticesPlaneVBO = 0;
    }

    if (g_normalsPlaneVBO)
    {
        glDeleteBuffers(1, &g_normalsPlaneVBO);

        g_normalsPlaneVBO = 0;
    }

    if (g_indicesPlaneVBO)
    {
        glDeleteBuffers(1, &g_indicesPlaneVBO);

        g_indicesPlaneVBO = 0;
    }

    if (g_planeVAO)
    {
        glDeleteVertexArrays(1, &g_planeVAO);

        g_planeVAO = 0;
    }

    //

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
	        EGL_STENCIL_SIZE, 8,
	        EGL_SAMPLE_BUFFERS, 1,
	        EGL_SAMPLES, 16,
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
