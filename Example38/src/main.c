/**
 * OpenGL 4 - Example 38
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
 * The used separable programs.
 */
static GLUSprogram g_vertexProgram;
static GLUSprogram g_fragmentProgram;

/**
 * The used shader program pipeline.
 */
static GLUSprogrampipeline g_programPipeline;

/**
 * The location of the vertex in the vertex shader program.
 */
static GLint g_vertexLocation;

/**
 * The VBO for the vertices.
 */
static GLuint g_verticesVBO;

/**
 * The VAO for the vertices and later normals etc..
 */
static GLuint g_vao;

GLUSboolean init(GLUSvoid)
{
    // Points of a triangle in normalized device coordinates.
    GLfloat points[] = { -0.5f, 0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.5f, 0.0f, 1.0f };

    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    // Load the source of the vertex and fragment shader.
    glusFileLoadText("../Example38/shader/simple.vert.glsl", &vertexSource);
    glusFileLoadText("../Example38/shader/red.frag.glsl", &fragmentSource);

    // Build the programs.
    glusProgramBuildSeparableFromSource(&g_vertexProgram, GL_VERTEX_SHADER, (const GLchar**) &vertexSource.text);
    glusProgramBuildSeparableFromSource(&g_fragmentProgram, GL_FRAGMENT_SHADER, (const GLchar**) &fragmentSource.text);

    // Destroy the text resources.
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    // Build the program pipeline.
    glusProgramPipelineBuild(&g_programPipeline, g_vertexProgram.program, 0, 0, 0, g_fragmentProgram.program);

    //

    // Retrieve the vertex location in the vertex program.
    g_vertexLocation = glGetAttribLocation(g_vertexProgram.program, "a_vertex");

    //

    // Create and bind the VBO for the vertices.
    glGenBuffers(1, &g_verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);

    // Transfer the vertices from CPU to GPU.
    glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(GLfloat), (GLfloat*) points, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //

    // Bind the program pipeline.
    glBindProgramPipeline(g_programPipeline.pipeline);

    // Create the VAO for the program pipeline.
    glGenVertexArrays(1, &g_vao);
    glBindVertexArray(g_vao);

    // Bind the only used VBO in this example.
    glBindBuffer(GL_ARRAY_BUFFER, g_verticesVBO);
    glVertexAttribPointer(g_vertexLocation, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(g_vertexLocation);

    //

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    glViewport(0, 0, width, height);
}

GLUSboolean update(GLUSfloat time)
{
    glClear(GL_COLOR_BUFFER_BIT);

    // This draws the triangle, having three points.
    glDrawArrays(GL_TRIANGLES, 0, 3);

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

	glBindVertexArray(0);

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_vao);

        g_vao = 0;
    }

    glBindProgramPipeline(0);

    glusProgramPipelineDestroy(&g_programPipeline);

    glUseProgram(0);

    glusProgramDestroy(&g_vertexProgram);
    glusProgramDestroy(&g_fragmentProgram);
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
    		EGL_CONTEXT_MINOR_VERSION, 1,
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
