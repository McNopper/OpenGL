/**
 * OpenGL 3 - Example 07
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 */

#include <math.h>
#include <stdio.h>

#include "GL/glus.h"

static GLUSprogram g_program;

//

static GLint g_projectionMatrixLocation;

static GLint g_modelViewMatrixLocation;

static GLint g_normalMatrixLocation;

//

static GLint g_lightDirectionLocation;

static GLint g_vertexLocation;

static GLint g_tangentLocation;

static GLint g_bitangentLocation;

static GLint g_normalLocation;

static GLint g_texCoordLocation;

//

static GLint g_textureLocation;

static GLint g_normalMapLocation;

//

static GLfloat g_viewMatrix[16];

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

static GLuint g_texture;

static GLuint g_normalMap;

//

static GLuint g_numberIndicesPlane;

GLUSboolean init(GLUSvoid)
{
    GLUSshape plane;

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLUStgaimage image;

    GLfloat normalMatrix[9];

    glusFileLoadText("../Example07/shader/normmap.vert.glsl", &vertexSource);
    glusFileLoadText("../Example07/shader/normmap.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_program, (const GLUSchar**) &vertexSource.text, 0, 0, 0, (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    //

    g_projectionMatrixLocation = glGetUniformLocation(g_program.program, "u_projectionMatrix");
    g_modelViewMatrixLocation = glGetUniformLocation(g_program.program, "u_modelViewMatrix");
    g_normalMatrixLocation = glGetUniformLocation(g_program.program, "u_normalMatrix");
    g_lightDirectionLocation = glGetUniformLocation(g_program.program, "u_lightDirection");

    // One texture for the color and one for the normals.
    g_textureLocation = glGetUniformLocation(g_program.program, "u_texture");
    g_normalMapLocation = glGetUniformLocation(g_program.program, "u_normalMap");

    g_vertexLocation = glGetAttribLocation(g_program.program, "a_vertex");
    // The tangent, bitangent and normal do define the tangent space. They are defined in object space, the inverse brings coordinates back to this tangent space.
    g_tangentLocation = glGetAttribLocation(g_program.program, "a_tangent");
    g_bitangentLocation = glGetAttribLocation(g_program.program, "a_bitangent");
    g_normalLocation = glGetAttribLocation(g_program.program, "a_normal");

    g_texCoordLocation = glGetAttribLocation(g_program.program, "a_texCoord");

    //

    glusImageLoadTga("rock_color.tga", &image);

    glGenTextures(1, &g_texture);
    glBindTexture(GL_TEXTURE_2D, g_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

    glusImageDestroyTga(&image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);


    glusImageLoadTga("rock_normal.tga", &image);

    glGenTextures(1, &g_normalMap);
    glBindTexture(GL_TEXTURE_2D, g_normalMap);
    glTexImage2D(GL_TEXTURE_2D, 0, image.format, image.width, image.height, 0, image.format, GL_UNSIGNED_BYTE, image.data);

    glusImageDestroyTga(&image);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glBindTexture(GL_TEXTURE_2D, 0);

    //

    glusShapeCreatePlanef(&plane, 1.5f);

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

    glusMatrix4x4LookAtf(g_viewMatrix, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    glUniformMatrix4fv(g_modelViewMatrixLocation, 1, GL_FALSE, g_viewMatrix);

    glusMatrix4x4ExtractMatrix3x3f(normalMatrix, g_viewMatrix);

    glUniformMatrix3fv(g_normalMatrixLocation, 1, GL_FALSE, normalMatrix);

    //

    // Activate and bind first ...
    glUniform1i(g_textureLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_texture);

    // .. and second texture.
    glUniform1i(g_normalMapLocation, 1);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, g_normalMap);

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    GLfloat projectionMatrix[16];

    glViewport(0, 0, width, height);

    glusMatrix4x4Perspectivef(projectionMatrix, 40.0f, (GLfloat) width / (GLfloat) height, 1.0f, 100.0f);

    glUniformMatrix4fv(g_projectionMatrixLocation, 1, GL_FALSE, projectionMatrix);
}

GLUSboolean update(GLUSfloat time)
{
    static GLfloat angle = 0.0;

    // Light start direction, which will be animated for seeing the effect.
    GLfloat lightDirection[3] = { 1.0, 1.0f, 1.0f };

    // Increase the angle depending on the time.
    angle += time;

    // Let the light move from right to left and vice versa.
    lightDirection[0] = 2.0f * cosf(angle);
    // Need to normalize the light given in world space.
    glusVector3Normalizef(lightDirection);

    // Bring the light to camera space.
    glusMatrix4x4MultiplyVector3f(lightDirection, g_viewMatrix, lightDirection);

    glUniform3fv(g_lightDirectionLocation, 1, (GLfloat*) lightDirection);

    glClear(GL_COLOR_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, g_numberIndicesPlane, GL_UNSIGNED_INT, 0);

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

    if (g_texture)
    {
        glActiveTexture(GL_TEXTURE0);

        glBindTexture(GL_TEXTURE_2D, 0);

        glDeleteTextures(1, &g_texture);

        g_texture = 0;
    }

    if (g_normalMap)
    {
        glActiveTexture(GL_TEXTURE1);

        glBindTexture(GL_TEXTURE_2D, 0);

        glDeleteTextures(1, &g_normalMap);

        g_normalMap = 0;
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
	        EGL_DEPTH_SIZE, 0,
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
