/**
 * OpenGL 3 - Example 26
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
 * Locations for the light properties.
 */
struct LightLocations
{
    GLint directionLocation;
    GLint ambientColorLocation;
    GLint diffuseColorLocation;
    GLint specularColorLocation;
};

static GLfloat g_viewMatrix[16];

static GLUSprogram g_program;

static GLUSprogram g_programFur;

//

static GLint g_projectionMatrixLocation;

static GLint g_modelViewMatrixLocation;

static GLint g_normalMatrixLocation;

//

static GLint g_projectionMatrixFurLocation;

static GLint g_modelViewMatrixFurLocation;

static GLint g_normalMatrixFurLocation;

static GLint g_lightDirectionFurLocation;

//

static GLint g_textureLocation;

static GLint g_textureFurColorLocation;

static GLint g_textureFurStrengthLocation;

//

static GLint g_vertexLocation;

static GLint g_normalLocation;

static GLint g_texCoordLocation;

//

static GLint g_vertexFurLocation;

static GLint g_normalFurLocation;

static GLint g_texCoordFurLocation;

/**
 * The locations for the light properties.
 */
static struct LightLocations g_light;

//

static GLuint g_textureFurColor;

static GLuint g_textureFurStrength;

//

static GLuint g_verticesVBO;

static GLuint g_normalsVBO;

static GLuint g_texCoordsVBO;

//

static GLuint g_vao;

static GLuint g_vaoFur;

//

static GLuint g_numberVertices;

GLUSboolean init(GLUSvoid)
{
    // This is a white light.
    struct LightProperties light = { { 1.0f, 1.0f, 1.0f }, { 0.3f, 0.3f, 0.3f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f, 1.0f, 1.0f } };

    GLUStextfile vertexSource;
    GLUStextfile geometrySource;
    GLUStextfile fragmentSource;

    GLUStgaimage image;

    GLUSshape bunnyShape;

    //

    glusFileLoadText("../Example26/shader/ambient_diffuse_texture.vert.glsl", &vertexSource);
    glusFileLoadText("../Example26/shader/ambient_diffuse_texture.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);


    glusFileLoadText("../Example26/shader/fur.vert.glsl", &vertexSource);
    glusFileLoadText("../Example26/shader/fur.geom.glsl", &geometrySource);
    glusFileLoadText("../Example26/shader/fur.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_programFur, (const GLUSchar**) &vertexSource.text, 0, 0, (const GLUSchar**) &geometrySource.text, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&geometrySource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_modelViewMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");

    g_light.directionLocation = glGetUniformLocation(g_program.program, "u_light.direction");
    g_light.ambientColorLocation = glGetUniformLocation(g_program.program, "u_light.ambientColor");
    g_light.diffuseColorLocation = glGetUniformLocation(g_program.program, "u_light.diffuseColor");
    g_light.specularColorLocation = glGetUniformLocation(g_program.program, "u_light.specularColor");

    g_textureLocation = glGetUniformLocation(g_program.program, "u_textureFurColor");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");
    g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");


    g_projectionMatrixFurLocation = glGetUniformLocation(g_programFur.program, "u_projectionMatrix");
    g_modelViewMatrixFurLocation = glGetUniformLocation(g_programFur.program, "u_modelViewMatrix");
    g_normalMatrixFurLocation = glGetUniformLocation(g_programFur.program, "u_normalMatrix");

    g_lightDirectionFurLocation = glGetUniformLocation(g_programFur.program, "u_lightDirection");

    g_textureFurColorLocation = glGetUniformLocation(g_programFur.program, "u_textureFurColor");
    g_textureFurStrengthLocation = glGetUniformLocation(g_programFur.program, "u_textureFurStrength");

    g_vertexFurLocation = glGetAttribLocation(g_programFur.program, "a_vertex");
    g_normalFurLocation = glGetAttribLocation(g_programFur.program, "a_normal");
    g_texCoordFurLocation = glGetAttribLocation(g_programFur.program, "a_texCoord");

    //

    // Color texture set up.

    glusImageLoadTga("tiger.tga", &image);

    glGenTextures(1, &g_textureFurColor);
    glBindTexture(GL_TEXTURE_2D, g_textureFurColor);

    glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

    // Mipmap generation is now included in OpenGL 3 and above
    glGenerateMipmap(GL_TEXTURE_2D);

    // Trilinear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glusImageDestroyTga(&image);

    // A simple noise texture influences the strength of each fur pixel.

    glusPerlinCreateNoise2D(&image, 64, 64, 0, 100.0f, 255.0f, 0.5f, 1);

    glGenTextures(1, &g_textureFurStrength);
    glBindTexture(GL_TEXTURE_2D, g_textureFurStrength);

    glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

    // Mipmap generation is now included in OpenGL 3 and above
    glGenerateMipmap(GL_TEXTURE_2D);

    // Trilinear filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    glusImageDestroyTga(&image);

    //

    // Use a helper function to load an wavefront object file.
    glusShapeLoadWavefront("bunny.obj", &bunnyShape);

    // This model does not have any texture coordinates, so generate them.
    glusShapeTexGenByAxesf(&bunnyShape, 2.0f, 0.0f, 2.0f, 0.0f, 0.0f, 0.0f);

    g_numberVertices = bunnyShape.numberVertices;

    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glBufferData(GL_ARRAY_BUFFER, bunnyShape.numberVertices * 4 * sizeof(GLfloat), (GLfloat*) bunnyShape.vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &g_normalsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glBufferData(GL_ARRAY_BUFFER, bunnyShape.numberVertices * 3 * sizeof(GLfloat), (GLfloat*) bunnyShape.normals, GL_STATIC_DRAW);

    glGenBuffers(1, &g_texCoordsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER, bunnyShape.numberVertices * 2 * sizeof(GLfloat), (GLfloat*) bunnyShape.texCoords, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glusShapeDestroyf(&bunnyShape);

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

    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
    glVertexAttribPointer(g_texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_texCoordLocation);


    glUseProgram(g_programFur.program);

    glGenVertexArrays(1, &g_vaoFur);
    glBindVertexArray(g_vaoFur);

    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_normalsVBO);
    glVertexAttribPointer(g_normalLocation, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_normalLocation);

    glBindBuffer(GL_ARRAY_BUFFER, g_texCoordsVBO);
    glVertexAttribPointer(g_texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_texCoordLocation);

    //

    glusMatrix4x4LookAtf(g_viewMatrix, 0.0f, 0.75f, 3.0f, 0.0f, 0.75f, 0.0f, 0.0f, 1.0f, 0.0f);

    //

    glusVector3Normalizef(light.direction);

    // Transform light to camera space, as it is currently in world space.
    glusMatrix4x4MultiplyVector3f(light.direction, g_viewMatrix, light.direction);

    glUseProgram(g_program.program);

    // Set up light ...
    glUniform3fv(g_light.directionLocation, 1, light.direction);
    glUniform4fv(g_light.ambientColorLocation, 1, light.ambientColor);
    glUniform4fv(g_light.diffuseColorLocation, 1, light.diffuseColor);
    glUniform4fv(g_light.specularColorLocation, 1, light.specularColor);

    // ... and the texture.

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_textureFurColor);
    glUniform1i(g_textureLocation, 0);


    glUseProgram(g_programFur.program);

    glUniform3fv(g_lightDirectionFurLocation, 1, light.direction);

    // Fur color and position textures.

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_textureFurColor);
    glUniform1i(g_textureFurColorLocation, 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_textureFurStrength);
    glUniform1i(g_textureFurStrengthLocation, 1);

    //

    glClearColor(0.8f, 0.8f, 0.8f, 0.0f);

    glClearDepth(1.0f);

    glEnable(GL_DEPTH_TEST);

    glEnable(GL_CULL_FACE);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    GLfloat projectionMatrix[16];

    glViewport(0, 0, width, height);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    glUseProgram(g_program.program);

    // Just pass the projection matrix. The final matrix is calculated in the shader.
    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);


    glUseProgram(g_programFur.program);

    // Just pass the projection matrix. The final matrix is calculated in the shader.
    glUniformMatrix4fv(g_projectionMatrixFurLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
    GLfloat modelViewMatrix[16];
    GLfloat normalMatrix[9];

    glusMatrix4x4Identityf(modelViewMatrix);

    glusMatrix4x4Multiplyf(modelViewMatrix, g_viewMatrix, modelViewMatrix);

    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, modelViewMatrix);

    //

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Normal rendering

    glUseProgram(g_program.program);

    glUniformMatrix4fv(g_modelViewMatrixLocation, 1, GL_FALSE, modelViewMatrix);
    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    glBindVertexArray(g_vao);

    glDrawArrays(GL_TRIANGLES, 0, g_numberVertices);

    // Fur rendering

    // Do not write to depth, so no fur layer will be discarded.
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);

    glUseProgram(g_programFur.program);

    glUniformMatrix4fv(g_modelViewMatrixFurLocation, 1, GL_FALSE, modelViewMatrix);
    glUniformMatrix3fv(g_normalMatrixFurLocation, 1, GL_FALSE, normalMatrix);

    glBindVertexArray(g_vaoFur);

    glDrawArrays(GL_TRIANGLES, 0, g_numberVertices);

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glBindTexture(GL_TEXTURE_2D, 0);

	if (g_textureFurColor)
    {
        glDeleteTextures(1, &g_textureFurColor);

        g_textureFurColor = 0;
    }

	if (g_textureFurStrength)
    {
        glDeleteTextures(1, &g_textureFurStrength);

        g_textureFurStrength = 0;
    }

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

    if (g_texCoordsVBO)
    {
        glDeleteBuffers(1, &g_texCoordsVBO);

        g_texCoordsVBO = 0;
    }

    glBindVertexArray(0);

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_vao);

        g_vao = 0;
    }

    if (g_vaoFur)
    {
        glDeleteVertexArrays(1, &g_vaoFur);

        g_vaoFur = 0;
    }

    glUseProgram(0);

    glusProgramDestroy(&g_program);

    glusProgramDestroy(&g_programFur);
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
	        EGL_SAMPLES, 8,
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
