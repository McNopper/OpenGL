/**
 * OpenGL 4.6 - Example 47
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * Demonstrates GPU-based 3D spatial colour sorting using a 3D texture as the
 * core data structure.
 *
 * An N^3 point cloud is initialised with all N^3 unique RGB8 lattice colours
 * (one per grid cell) placed in a random order.  An odd-even transposition
 * sort compute shader operates directly on the image3D along each axis:
 *
 *   X-axis pass  ->  sort by R  ->  R increases with x
 *   Y-axis pass  ->  sort by G  ->  G increases with y
 *   Z-axis pass  ->  sort by B  ->  B increases with z
 *
 * Repeating these three passes GRID_N times converges to the RGB cube where
 * texel (x,y,z) holds colour (x,y,z)*255/(N-1).  The space diagonal of the
 * cube runs from rgb(0,0,0) at corner (0,0,0) to rgb(255,255,255) at corner
 * (N-1,N-1,N-1).
 *
 * The sort is animated step by step so the cube can be seen crystallising
 * out of the initial random colour noise.
 *
 * Controls:
 *   Space  -  start the sort animation
 *   + / -  -  increase / decrease delay between steps (default 100 ms)
 *   R      -  reshuffle to a new random order
 */

#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "GL/glus.h"

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

// Grid dimension — queried from hardware limits at startup.
// Injected into the sort shader as #define GRID_N <n> before compilation.
static GLint g_gridN = 32; // default; overwritten in init()

// Sort dispatches per rendered frame.  3 = one full X->Y->Z cycle per frame,
// which makes the crystallisation animation smooth without being too fast.
#define STEPS_PER_FRAME 3

// Camera orbit speed (degrees per second).
#define ORBIT_SPEED 18.0f

//
// Sort compute program.
//

static GLUSprogram g_sortProgram;

static GLint g_sort_axisLoc;
static GLint g_sort_passLoc;

//
// Render program (attribute-less point cloud).
//

static GLUSprogram g_renderProgram;

static GLint g_render_mvpMatrixLoc;
static GLint g_render_gridNLoc;
static GLint g_render_pointSizeLoc;
static GLint g_render_colorTextureLoc;

//
// The 3D texture - the core data structure.
//

static GLuint g_colorTexture = 0;

// Empty VAO required by the core profile for attribute-less draws.
static GLuint g_vao = 0;

//
// Sort animation state.
//

static GLint     g_sortStep    = 0;
static GLboolean g_sorting     = GLUS_FALSE;
static GLboolean g_sorted      = GLUS_FALSE;
static GLint     g_stepDelayMs = 100;  // ms between steps; 0 = STEPS_PER_FRAME/frame
static GLfloat   g_accumTime   = 0.0f; // accumulated time for delay mode (seconds)

//
// Camera.
//

static GLfloat g_orbitAngle = 0.0f;

static GLint g_windowWidth  = WINDOW_WIDTH;
static GLint g_windowHeight = WINDOW_HEIGHT;

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

static void swapTexel(GLubyte* data, GLuint i, GLuint j)
{
    GLubyte tmp[4];
    memcpy(tmp, data + i * 4, 4);
    memcpy(data + i * 4, data + j * 4, 4);
    memcpy(data + j * 4, tmp, 4);
}

// Fill data with all N^3 unique RGB8 lattice colours then Fisher-Yates shuffle.
static void generateShuffledColors(GLubyte* data, GLuint n)
{
    GLuint x, y, z, i, j;

    if (n < 2)
    {
        return;
    }

    for (z = 0; z < n; z++)
    {
        for (y = 0; y < n; y++)
        {
            for (x = 0; x < n; x++)
            {
                i               = z * n * n + y * n + x;
                data[i * 4 + 0] = (GLubyte)(x * 255 / (n - 1)); // R -> X
                data[i * 4 + 1] = (GLubyte)(y * 255 / (n - 1)); // G -> Y
                data[i * 4 + 2] = (GLubyte)(z * 255 / (n - 1)); // B -> Z
                data[i * 4 + 3] = 255;
            }
        }
    }

    for (i = n * n * n - 1; i > 0; i--)
    {
        j = (GLuint)((GLfloat)(i + 1) * glusRandomUniformf(0.0f, 1.0f));
        if (j > i)
        {
            j = i;
        }
        swapTexel(data, i, j);
    }
}

// Query hardware limits and derive a suitable grid dimension.
// GRID_N = 32 is a good default (32^3 = 32 768 points, fits in any GPU's
// shared memory, and 32 odd-even passes guarantee convergence of each line).
static GLint queryIdealGridN(void)
{
    GLint maxLocalX, maxInvocations;
    GLint n, p;

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &maxLocalX);
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &maxInvocations);

    n = 32;
    if (n > maxLocalX)
    {
        n = maxLocalX;
    }
    if (n > maxInvocations)
    {
        n = maxInvocations;
    }

    // Round down to nearest power of two (required for odd-even pairing).
    p = 1;
    while (p * 2 <= n)
    {
        p *= 2;
    }
    n = p;

    printf("maxLocalX=%d  maxInvocations=%d  -> g_gridN=%d\n",
           maxLocalX, maxInvocations, n);

    return n;
}

// Inject "#define GRID_N <n>" after the "#version ..." first line of source.
// Returns a newly malloc'd string; caller must free() it.
static GLUSchar* injectDefine(const GLUSchar* source, GLint gridN)
{
    char        define[64];
    const char* firstNewline;
    size_t      prefixLen, defineLen, restLen;
    GLUSchar*   patched;

    snprintf(define, sizeof(define), "#define GRID_N %d\n", (int)gridN);

    firstNewline = strchr(source, '\n');
    prefixLen    = firstNewline ? (size_t)(firstNewline - source + 1) : 0;
    defineLen    = strlen(define);
    restLen      = strlen(source + prefixLen);

    patched = (GLUSchar*)malloc(prefixLen + defineLen + restLen + 1);
    if (!patched)
    {
        return NULL;
    }

    memcpy(patched, source, prefixLen);
    memcpy(patched + prefixLen, define, defineLen);
    memcpy(patched + prefixLen + defineLen, source + prefixLen, restLen + 1);

    return patched;
}

static void resetSortState(void)
{
    g_sortStep  = 0;
    g_sorting   = GLUS_FALSE;
    g_sorted    = GLUS_FALSE;
    g_accumTime = 0.0f;
}

// -----------------------------------------------------------------------
// Character callback — handles typed characters; used for +/- speed control
// because GLFW key codes do not map to '+' on all keyboard layouts.
// -----------------------------------------------------------------------

static GLUSvoid charInput(GLFWwindow* window, GLUSuint codepoint)
{
    (void)window;

    if (codepoint == '+' && g_stepDelayMs < 1000)
    {
        g_stepDelayMs += 50;
        glusLogPrint(GLUS_LOG_INFO, "Step delay: %d ms", g_stepDelayMs);
    }

    if (codepoint == '-' && g_stepDelayMs > 0)
    {
        g_stepDelayMs -= 50;
        if (g_stepDelayMs < 0)
        {
            g_stepDelayMs = 0;
        }
        glusLogPrint(GLUS_LOG_INFO, "Step delay: %d ms%s",
                     g_stepDelayMs, g_stepDelayMs == 0 ? " (smooth)" : "");
    }
}

// -----------------------------------------------------------------------
// Key callback
// -----------------------------------------------------------------------

GLUSvoid key(const GLUSboolean pressed, const GLUSint k)
{
    if (!pressed)
    {
        return;
    }

    // Space: start sort if idle, or restart once finished.
    if (k == 32 && !g_sorting)
    {
        g_sorting   = GLUS_TRUE;
        g_sorted    = GLUS_FALSE;
        g_sortStep  = 0;
        g_accumTime = 0.0f;
    }

    // R: reshuffle the texture; press Space to start sorting.
    if (k == 'r' || k == 'R')
    {
        GLubyte* data = (GLubyte*)malloc((size_t)(g_gridN * g_gridN * g_gridN) * 4);

        if (data)
        {
            generateShuffledColors(data, (GLuint)g_gridN);

            glBindTexture(GL_TEXTURE_3D, g_colorTexture);
            glTexSubImage3D(GL_TEXTURE_3D, 0,
                            0, 0, 0,
                            g_gridN, g_gridN, g_gridN,
                            GL_RGBA, GL_UNSIGNED_BYTE, data);
            glBindTexture(GL_TEXTURE_3D, 0);

            free(data);
        }

        resetSortState();
    }
}

// -----------------------------------------------------------------------
// Init
// -----------------------------------------------------------------------

GLUSboolean init(GLUSvoid)
{
    GLUStextfile sortSource;
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    GLubyte* colorData;

    glusRandomSetSeed(42);

    // Query hardware for the ideal GRID_N before any shader compilation.
    g_gridN = queryIdealGridN();

    //
    // Sort compute shader — GRID_N injected as a #define before compilation.
    //

    glusFileLoadText("../Example47/shader/sort.comp.glsl", &sortSource);

    {
        GLUSchar* patchedSource = injectDefine(sortSource.text, g_gridN);
        glusProgramBuildComputeFromSource(&g_sortProgram, (const GLUSchar**)&patchedSource);
        free(patchedSource);
    }

    glusFileDestroyText(&sortSource);

    glUseProgram(g_sortProgram.program);

    g_sort_axisLoc = glGetUniformLocation(g_sortProgram.program, "u_axis");
    g_sort_passLoc = glGetUniformLocation(g_sortProgram.program, "u_pass");

    glUseProgram(0);

    //
    // Render program.
    //

    glusFileLoadText("../Example47/shader/render.vert.glsl", &vertexSource);
    glusFileLoadText("../Example47/shader/render.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_renderProgram,
                               (const GLUSchar**)&vertexSource.text,
                               0, 0, 0,
                               (const GLUSchar**)&fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    glUseProgram(g_renderProgram.program);

    g_render_mvpMatrixLoc    = glGetUniformLocation(g_renderProgram.program, "u_mvpMatrix");
    g_render_gridNLoc        = glGetUniformLocation(g_renderProgram.program, "u_gridN");
    g_render_pointSizeLoc    = glGetUniformLocation(g_renderProgram.program, "u_pointSize");
    g_render_colorTextureLoc = glGetUniformLocation(g_renderProgram.program, "u_colorTexture");

    glUniform1ui(g_render_gridNLoc, (GLuint)g_gridN);
    glUniform1f(g_render_pointSizeLoc, 8.0f);
    glUniform1i(g_render_colorTextureLoc, 0);

    glUseProgram(0);

    //
    // Create the 3D texture - the core data structure.
    //

    colorData = (GLubyte*)malloc((size_t)(g_gridN * g_gridN * g_gridN) * 4);

    if (!colorData)
    {
        printf("Could not allocate colour data.\n");
        return GLUS_FALSE;
    }

    generateShuffledColors(colorData, (GLuint)g_gridN);

    glGenTextures(1, &g_colorTexture);
    glBindTexture(GL_TEXTURE_3D, g_colorTexture);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA8,
                 g_gridN, g_gridN, g_gridN,
                 0, GL_RGBA, GL_UNSIGNED_BYTE, colorData);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_3D, 0);

    free(colorData);

    //
    // Empty VAO for attribute-less point cloud rendering.
    //

    glGenVertexArrays(1, &g_vao);

    //
    // Global OpenGL state.
    //

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);

    glusLogPrint(GLUS_LOG_INFO, "Controls:");
    glusLogPrint(GLUS_LOG_INFO, "  Space : start sort");
    glusLogPrint(GLUS_LOG_INFO, "  +/-   : slower/faster step delay (current: %d ms)", g_stepDelayMs);
    glusLogPrint(GLUS_LOG_INFO, "  R     : reshuffle");

    // Start sorting immediately on launch.
    g_sorting = GLUS_TRUE;

    return GLUS_TRUE;
}

// -----------------------------------------------------------------------
// Reshape
// -----------------------------------------------------------------------

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    g_windowWidth  = width;
    g_windowHeight = height;

    glViewport(0, 0, width, height);
}

// -----------------------------------------------------------------------
// Update
// -----------------------------------------------------------------------

GLUSboolean update(GLUSfloat time)
{
    GLfloat viewMatrix[16];
    GLfloat projectionMatrix[16];
    GLfloat mvpMatrix[16];
    GLfloat eyeX, eyeZ;

    g_orbitAngle += ORBIT_SPEED * time;
    if (g_orbitAngle >= 360.0f)
    {
        g_orbitAngle -= 360.0f;
    }

    //
    // Sort pass: advance sort by the number of steps determined by the current mode.
    //   g_stepDelayMs > 0  → one step per delay period (+/- keys)
    //   g_stepDelayMs == 0 → STEPS_PER_FRAME steps per frame (smooth animation)
    //
    if (g_sorting)
    {
        GLint stepsThisFrame;
        // One visible step = one full axis sort (GRID_N single-pass dispatches).
        // Axis cycles X→Y→Z→X→... so the animation shows each axis in turn.
        // Total visible steps = GRID_N cycles × 3 axes.
        GLint totalSteps = g_gridN * 3;

        if (g_stepDelayMs > 0)
        {
            g_accumTime += time;
            if (g_accumTime >= g_stepDelayMs * 0.001f)
            {
                stepsThisFrame = 1;
                g_accumTime    = 0.0f;
            }
            else
            {
                stepsThisFrame = 0;
            }
        }
        else
        {
            stepsThisFrame = STEPS_PER_FRAME;
        }

        if (stepsThisFrame > 0)
        {
            GLint s;

            glUseProgram(g_sortProgram.program);

            glBindImageTexture(0, g_colorTexture, 0,
                               GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

            for (s = 0; s < stepsThisFrame && g_sorting; s++)
            {
                // One visible step = one full axis sort: GRID_N single-pass dispatches.
                // Axis cycles X(0)→Y(1)→Z(2)→X(0)→...
                int axis = g_sortStep % 3;
                int p;

                for (p = 0; p < g_gridN; p++)
                {
                    glUniform1i(g_sort_axisLoc, axis);
                    glUniform1i(g_sort_passLoc, p);

                    glDispatchCompute(g_gridN, g_gridN, 1);

                    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                }

                g_sortStep++;

                if (g_sortStep >= totalSteps)
                {
                    g_sorting = GLUS_FALSE;
                    g_sorted  = GLUS_TRUE;
                }
            }

            glBindImageTexture(0, 0, 0, GL_TRUE, 0, GL_READ_WRITE, GL_RGBA8);

            glUseProgram(0);
        }
    }

    // Ensure the last image writes are visible to the texture sampler.
    glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);

    //
    // Render pass: draw point cloud.
    //

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    {
        GLfloat rad = g_orbitAngle * GLUS_PI / 180.0f;
        eyeX        = 3.5f * sinf(rad);
        eyeZ        = 3.5f * cosf(rad);
    }

    glusMatrix4x4LookAtf(viewMatrix,
                         eyeX, 2.5f, eyeZ,
                         0.0f, 0.0f, 0.0f,
                         0.0f, 1.0f, 0.0f);

    glusMatrix4x4Perspectivef(projectionMatrix,
                              60.0f,
                              (GLfloat)g_windowWidth / (GLfloat)g_windowHeight,
                              0.1f, 100.0f);

    glusMatrix4x4Multiplyf(mvpMatrix, projectionMatrix, viewMatrix);

    glUseProgram(g_renderProgram.program);

    glUniformMatrix4fv(g_render_mvpMatrixLoc, 1, GL_FALSE, mvpMatrix);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, g_colorTexture);

    glBindVertexArray(g_vao);
    glDrawArrays(GL_POINTS, 0, g_gridN * g_gridN * g_gridN);
    glBindVertexArray(0);

    glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE0);

    glUseProgram(0);

    return GLUS_TRUE;
}

// -----------------------------------------------------------------------
// Terminate
// -----------------------------------------------------------------------

GLUSvoid terminate(GLUSvoid)
{
    glBindTexture(GL_TEXTURE_3D, 0);

    if (g_colorTexture)
    {
        glDeleteTextures(1, &g_colorTexture);
        g_colorTexture = 0;
    }

    glBindVertexArray(0);

    if (g_vao)
    {
        glDeleteVertexArrays(1, &g_vao);
        g_vao = 0;
    }

    glUseProgram(0);

    glusProgramDestroy(&g_sortProgram);
    glusProgramDestroy(&g_renderProgram);
}

// -----------------------------------------------------------------------
// Entry point
// -----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    EGLint eglConfigAttributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE};

    EGLint eglContextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 6,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
        EGL_NONE};

    // Show GL driver debug messages (shader errors, invalid operations, etc.).
    glusLogSetLevel(GLUS_LOG_DEBUG);

    glusWindowSetInitFunc(init);
    glusWindowSetReshapeFunc(reshape);
    glusWindowSetUpdateFunc(update);
    glusWindowSetTerminateFunc(terminate);
    glusWindowSetKeyFunc(key);

    if (!glusWindowCreate("GLUS Example Window", WINDOW_WIDTH, WINDOW_HEIGHT,
                          GLUS_FALSE, GLUS_TRUE,
                          eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Could not create window!\n");
        return -1;
    }

    glfwSetCharCallback(glfwGetCurrentContext(), charInput);

    glusWindowRun();

    return 0;
}