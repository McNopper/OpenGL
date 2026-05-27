/**
 * OpenGL 4.6 - Example 51
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * 3D Gaussian Splatting renderer implementing the KHR_gaussian_splatting
 * glTF 2.0 extension (https://github.com/KhronosGroup/glTF/tree/main/extensions/2.0/Khronos/KHR_gaussian_splatting).
 *
 * Splats are sorted back-to-front each frame using a GPU bitonic sort
 * (adapted from Example 47) and rendered as instanced screen-space quads
 * with full spherical-harmonics colour evaluation (band 0-3) including
 * Wigner-D rotation for nodes with transforms.
 *
 * Usage:  Example51 [path/to/model.gltf]
 * Default model: ../Binaries/lego.gltf
 *
 * Controls:
 *   Left / Right  - orbit horizontally
 *   Up   / Down   - orbit vertically
 *   Page Up/Down  - zoom in / out
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CGLTF_IMPLEMENTATION
#include "cgltf.h"

#include "GL/glus.h"

// -----------------------------------------------------------------------
// Constants
// -----------------------------------------------------------------------

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define SORT_LOCAL_SIZE 256

#define ORBIT_SPEED 0.5f

#define ORBIT_DELTA_THETA 0.05f
#define ORBIT_DELTA_PHI 0.05f
#define ORBIT_DELTA_RADIUS 0.5f

// -----------------------------------------------------------------------
// Global GL objects
// -----------------------------------------------------------------------

static GLUSprogram g_depthProgram;
static GLUSprogram g_sortProgram;
static GLUSprogram g_splatProgram;

// Uniform locations for the depth compute pass.
static GLint g_depth_worldMatrixLoc;
static GLint g_depth_numSplatsLoc;
static GLint g_depth_numSplatsPaddedLoc;

// Uniform locations for the bitonic sort pass.
static GLint g_sort_jLoc;
static GLint g_sort_kLoc;

// GL buffer objects.
static GLuint g_splatSSBO = 0; // binding 0: splat float data (read-only)
static GLuint g_indexSSBO = 0; // binding 1: sorted indices
static GLuint g_depthSSBO = 0; // binding 2: Euclidean distances
static GLuint g_modelSSBO = 0; // binding 3: world matrix + Wigner-D matrices
static GLuint g_worldUBO  = 0; // UBO binding 0: projection, view, focal, viewport, camPos

// Quad VAO for instanced splat rendering.
static GLuint g_quadVAO = 0;
static GLuint g_quadVBO = 0;

// -----------------------------------------------------------------------
// Splat model state (filled in init)
// -----------------------------------------------------------------------

static GLuint g_numSplats       = 0;
static GLuint g_numSplatsPadded = 0; // next power of 2 >= g_numSplats
static GLint  g_shDegree        = 0;
static GLuint g_splatStride     = 0; // floats per splat

// World matrix for the node (used each frame by the depth compute pass).
static GLfloat g_worldMatrix[16];

// -----------------------------------------------------------------------
// Camera orbit state
// -----------------------------------------------------------------------

static GLfloat g_theta  = 0.0f; // azimuth (radians)
static GLfloat g_phi    = 0.3f; // elevation (radians)
static GLfloat g_radius = 5.0f; // distance from origin

// glTF model path (may be overridden by argv[1]).
static const char* g_gltfPath = "../Binaries/lego.gltf";

// Current window size (updated in reshape).
static GLint g_windowWidth  = WINDOW_WIDTH;
static GLint g_windowHeight = WINDOW_HEIGHT;

// Cached projection matrix (set in reshape, reused in update).
static GLfloat g_projMatrix[16];

// -----------------------------------------------------------------------
// WorldData UBO layout (matches std140 in the vertex shader)
// -----------------------------------------------------------------------
typedef struct
{
    GLfloat projMatrix[16]; // offset   0
    GLfloat viewMatrix[16]; // offset  64
    GLfloat focal[2];       // offset 128
    GLfloat viewport[2];    // offset 136
    GLfloat camPos[4];      // offset 144  (vec4; w unused)
} WorldData;                // total: 160 bytes

// -----------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------

// Returns the smallest power of two that is >= n.
static GLuint nextPowerOfTwo(GLuint n)
{
    GLuint p = 1;
    while (p < n)
    {
        p <<= 1;
    }
    return p;
}

// Compute the number of floats per splat from the SH degree.
static GLuint strideForDegree(GLint degree)
{
    // base: 3(pos) + 4(rot) + 3(scale) + 1(opacity) + 3(sh0) = 14
    // +degree1: 9, +degree2: 15, +degree3: 21
    static const GLuint table[] = {14, 23, 38, 59};
    if (degree < 0 || degree > 3)
    {
        return 59;
    }
    return table[degree];
}

// Inject "#define SPLAT_STRIDE S\n#define SH_DEGREE D\n" after the first
// line (#version ...) of source.  Returns a newly malloc'd string; caller
// must free() it.  Only injects SH_DEGREE when injectDegree is non-zero.
static GLUSchar* injectSplatDefines(const GLUSchar* source, GLuint stride, GLint degree, int injectDegree)
{
    char        define[128];
    const char* firstNewline;
    size_t      prefixLen, defineLen, restLen;
    GLUSchar*   patched;

    if (injectDegree)
    {
        snprintf(define, sizeof(define), "#define SPLAT_STRIDE %uu\n#define SH_DEGREE %d\n", stride, (int)degree);
    }
    else
    {
        snprintf(define, sizeof(define), "#define SPLAT_STRIDE %uu\n", stride);
    }

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

// -----------------------------------------------------------------------
// Key callback
// -----------------------------------------------------------------------

GLUSvoid key(const GLUSboolean pressed, const GLUSint k)
{
    if (!pressed)
    {
        return;
    }

    if (k == GLFW_KEY_LEFT)
    {
        g_theta -= ORBIT_DELTA_THETA;
    }
    if (k == GLFW_KEY_RIGHT)
    {
        g_theta += ORBIT_DELTA_THETA;
    }
    if (k == GLFW_KEY_UP)
    {
        g_phi += ORBIT_DELTA_PHI;
    }
    if (k == GLFW_KEY_DOWN)
    {
        g_phi -= ORBIT_DELTA_PHI;
    }
    if (k == GLFW_KEY_PAGE_UP)
    {
        g_radius -= ORBIT_DELTA_RADIUS;
    }
    if (k == GLFW_KEY_PAGE_DOWN)
    {
        g_radius += ORBIT_DELTA_RADIUS;
    }

    // Clamp elevation so the camera never flip past the poles.
    if (g_phi > 1.5f)
    {
        g_phi = 1.5f;
    }
    if (g_phi < -1.5f)
    {
        g_phi = -1.5f;
    }
    if (g_radius < 0.5f)
    {
        g_radius = 0.5f;
    }
}

// -----------------------------------------------------------------------
// Init callback
// -----------------------------------------------------------------------

GLUSboolean init(GLUSvoid)
{
    cgltf_options    cgltfOpts = {0};
    cgltf_data*      gltfData  = NULL;
    cgltf_result     cgltfRes;
    cgltf_primitive* prim;
    cgltf_accessor*  posAcc;
    cgltf_node*      splatNode;
    cgltf_size       ai;

    GLUStextfile depthSource, sortSource, vertSource, fragSource;
    GLUSchar*    patchedDepth;
    GLUSchar*    patchedVert;

    GLfloat rotation[9];
    GLfloat wigner1[9];
    GLfloat wigner2[25];
    GLfloat wigner3[49];

    const char* bvData;
    size_t      bvSize;
    GLuint      byteStride;

    // Quad corners in NDC.
    static const GLfloat quadVerts[8] = {
        -1.0f, -1.0f,
        1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f};

    //
    // Load the glTF model.
    //
    cgltfRes = cgltf_parse_file(&cgltfOpts, g_gltfPath, &gltfData);
    if (cgltfRes != cgltf_result_success)
    {
        glusLogPrint(GLUS_LOG_ERROR, "Failed to parse glTF: %s", g_gltfPath);
        return GLUS_FALSE;
    }
    cgltfRes = cgltf_load_buffers(&cgltfOpts, gltfData, g_gltfPath);
    if (cgltfRes != cgltf_result_success)
    {
        glusLogPrint(GLUS_LOG_ERROR, "Failed to load glTF buffers: %s", g_gltfPath);
        cgltf_free(gltfData);
        return GLUS_FALSE;
    }

    if (gltfData->meshes_count == 0 || gltfData->meshes[0].primitives_count == 0)
    {
        glusLogPrint(GLUS_LOG_ERROR, "glTF has no meshes or primitives: %s", g_gltfPath);
        cgltf_free(gltfData);
        return GLUS_FALSE;
    }
    prim = &gltfData->meshes[0].primitives[0];

    //
    // Detect SH degree.
    //
    g_shDegree = 0;
    for (ai = 0; ai < prim->attributes_count; ai++)
    {
        const char* name = prim->attributes[ai].name;
        if (strstr(name, "SH_DEGREE_3"))
        {
            g_shDegree = 3;
            break;
        }
        else if (strstr(name, "SH_DEGREE_2"))
        {
            g_shDegree = 2;
        }
        else if (strstr(name, "SH_DEGREE_1") && g_shDegree < 1)
        {
            g_shDegree = 1;
        }
    }
    g_splatStride = strideForDegree(g_shDegree);

    glusLogPrint(GLUS_LOG_INFO, "SH degree: %d  stride: %u floats/splat", g_shDegree, g_splatStride);

    //
    // Find POSITION accessor and upload splat buffer.
    //
    posAcc = NULL;
    for (ai = 0; ai < prim->attributes_count; ai++)
    {
        if (strcmp(prim->attributes[ai].name, "POSITION") == 0)
        {
            posAcc = prim->attributes[ai].data;
            break;
        }
    }
    if (!posAcc)
    {
        glusLogPrint(GLUS_LOG_ERROR, "No POSITION attribute in glTF primitive.");
        cgltf_free(gltfData);
        return GLUS_FALSE;
    }

    g_numSplats       = (GLuint)posAcc->count;
    g_numSplatsPadded = nextPowerOfTwo(g_numSplats);

    // Ensure the padded count is at least one full compute workgroup so the
    // integer division in glDispatchCompute(padded / SORT_LOCAL_SIZE, ...)
    // never produces zero work groups for small models.
    if (g_numSplatsPadded < SORT_LOCAL_SIZE)
    {
        g_numSplatsPadded = SORT_LOCAL_SIZE;
    }

    byteStride = (GLuint)(posAcc->buffer_view->stride);
    if (byteStride == 0)
    {
        byteStride = g_splatStride * sizeof(GLfloat);
    }

    bvData = (const char*)posAcc->buffer_view->buffer->data + posAcc->buffer_view->offset;
    bvSize = posAcc->buffer_view->size;

    glusLogPrint(GLUS_LOG_INFO, "Splat count: %u  padded: %u  byteStride: %u",
                 g_numSplats, g_numSplatsPadded, byteStride);

    glGenBuffers(1, &g_splatSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_splatSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, (GLsizeiptr)bvSize, bvData, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, g_splatSSBO);

    //
    // Index and depth SSBOs sized to padded count.
    //
    glGenBuffers(1, &g_indexSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_indexSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 (GLsizeiptr)(g_numSplatsPadded * sizeof(GLuint)), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, g_indexSSBO);

    glGenBuffers(1, &g_depthSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_depthSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER,
                 (GLsizeiptr)(g_numSplatsPadded * sizeof(GLfloat)), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, g_depthSSBO);

    //
    // Node world matrix and Wigner-D rotation matrices.
    //
    glusMatrix4x4Identityf(g_worldMatrix);

    splatNode = NULL;
    {
        cgltf_size ni;
        for (ni = 0; ni < gltfData->nodes_count; ni++)
        {
            if (gltfData->nodes[ni].mesh == &gltfData->meshes[0])
            {
                splatNode = &gltfData->nodes[ni];
                break;
            }
        }
    }
    if (splatNode)
    {
        cgltf_node_transform_world(splatNode, g_worldMatrix);
    }

    // Extract normalized rotation columns from the upper-left 3×3 of the
    // world matrix (column-major) for use with the Wigner-D recurrence.
    {
        int col;
        for (col = 0; col < 3; col++)
        {
            GLfloat cx  = g_worldMatrix[col * 4 + 0];
            GLfloat cy  = g_worldMatrix[col * 4 + 1];
            GLfloat cz  = g_worldMatrix[col * 4 + 2];
            GLfloat len = sqrtf(cx * cx + cy * cy + cz * cz);
            if (len > 1e-6f)
            {
                cx /= len;
                cy /= len;
                cz /= len;
            }
            rotation[col * 3 + 0] = cx;
            rotation[col * 3 + 1] = cy;
            rotation[col * 3 + 2] = cz;
        }
    }

    // Build Wigner-D matrices (identity if no rotation / degree 0).
    memset(wigner1, 0, sizeof(wigner1));
    wigner1[0] = wigner1[4] = wigner1[8] = 1.0f;
    memset(wigner2, 0, sizeof(wigner2));
    wigner2[0] = wigner2[6] = wigner2[12] = wigner2[18] = wigner2[24] = 1.0f;
    memset(wigner3, 0, sizeof(wigner3));
    wigner3[0] = wigner3[8] = wigner3[16] = wigner3[24] = wigner3[32] = wigner3[40] = wigner3[48] = 1.0f;

    if (g_shDegree >= 1)
    {
        glusSHBuildRotation1f(wigner1, rotation);
    }
    if (g_shDegree >= 2)
    {
        glusSHBuildRotation2f(wigner2, wigner1);
    }
    if (g_shDegree >= 3)
    {
        glusSHBuildRotation3f(wigner3, wigner1, wigner2);
    }

    // Upload ModelData SSBO: worldMatrix(64B) | wigner1(36B) | wigner2(100B) | wigner3(196B).
    glGenBuffers(1, &g_modelSSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, g_modelSSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, 396, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 64, g_worldMatrix);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 64, 36, wigner1);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 100, 100, wigner2);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, 200, 196, wigner3);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, g_modelSSBO);

    cgltf_free(gltfData);

    //
    // WorldData UBO.
    //
    glGenBuffers(1, &g_worldUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, g_worldUBO);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(WorldData), NULL, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 0, g_worldUBO);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //
    // Depth compute shader with SPLAT_STRIDE injected.
    //
    glusFileLoadText("../Example51/shader/depth.comp.glsl", &depthSource);
    patchedDepth = injectSplatDefines(depthSource.text, g_splatStride, g_shDegree, 0);
    glusProgramBuildComputeFromSource(&g_depthProgram, (const GLUSchar**)&patchedDepth);
    free(patchedDepth);
    glusFileDestroyText(&depthSource);

    glUseProgram(g_depthProgram.program);
    g_depth_worldMatrixLoc     = glGetUniformLocation(g_depthProgram.program, "u_worldMatrix");
    g_depth_numSplatsLoc       = glGetUniformLocation(g_depthProgram.program, "u_numSplats");
    g_depth_numSplatsPaddedLoc = glGetUniformLocation(g_depthProgram.program, "u_numSplatsPadded");
    glUseProgram(0);

    //
    // Bitonic sort compute shader.
    //
    glusFileLoadText("../Example51/shader/sort.comp.glsl", &sortSource);
    glusProgramBuildComputeFromSource(&g_sortProgram, (const GLUSchar**)&sortSource.text);
    glusFileDestroyText(&sortSource);

    glUseProgram(g_sortProgram.program);
    g_sort_jLoc = glGetUniformLocation(g_sortProgram.program, "u_j");
    g_sort_kLoc = glGetUniformLocation(g_sortProgram.program, "u_k");
    glUseProgram(0);

    //
    // Splat render program with SPLAT_STRIDE and SH_DEGREE injected.
    //
    glusFileLoadText("../Example51/shader/splat.vert.glsl", &vertSource);
    glusFileLoadText("../Example51/shader/splat.frag.glsl", &fragSource);

    patchedVert = injectSplatDefines(vertSource.text, g_splatStride, g_shDegree, 1);
    glusProgramBuildFromSource(&g_splatProgram,
                               (const GLUSchar**)&patchedVert,
                               0, 0, 0,
                               (const GLUSchar**)&fragSource.text);
    free(patchedVert);
    glusFileDestroyText(&vertSource);
    glusFileDestroyText(&fragSource);

    //
    // Quad VAO.
    //
    glGenVertexArrays(1, &g_quadVAO);
    glBindVertexArray(g_quadVAO);

    glGenBuffers(1, &g_quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerts), quadVerts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    //
    // Global GL state.
    //
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA); // premultiplied alpha

    // Suppress driver notification messages; keep errors and warnings.
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);

    glusLogPrint(GLUS_LOG_INFO, "Controls: Left/Right = orbit H  Up/Down = orbit V  PageUp/Down = zoom");

    return GLUS_TRUE;
}

// -----------------------------------------------------------------------
// Reshape callback
// -----------------------------------------------------------------------

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    g_windowWidth  = width;
    g_windowHeight = height;

    glViewport(0, 0, width, height);

    glusMatrix4x4Identityf(g_projMatrix);
    glusMatrix4x4Perspectivef(g_projMatrix, 45.0f, (GLUSfloat)width / (GLUSfloat)height, 0.1f, 1000.0f);
}

// -----------------------------------------------------------------------
// Update callback (called every frame)
// -----------------------------------------------------------------------

GLUSboolean update(GLUSfloat time)
{
    WorldData wd;
    GLfloat   viewMatrix[16];
    GLfloat   cx, cy, cz;
    GLuint    k, j;

    //
    // Auto-orbit: rotate around the scene.
    //
    g_theta += time * ORBIT_SPEED;

    //
    // Camera orbit.
    //
    cx = g_radius * cosf(g_phi) * sinf(g_theta);
    cy = g_radius * sinf(g_phi);
    cz = g_radius * cosf(g_phi) * cosf(g_theta);

    glusMatrix4x4Identityf(viewMatrix);
    glusMatrix4x4LookAtf(viewMatrix, cx, cy, cz, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);

    //
    // Upload WorldData UBO.
    //
    memcpy(wd.projMatrix, g_projMatrix, 64);
    memcpy(wd.viewMatrix, viewMatrix, 64);
    // Focal lengths: projMatrix[0]*w/2 and projMatrix[5]*h/2.
    wd.focal[0]    = g_projMatrix[0] * g_windowWidth * 0.5f;
    wd.focal[1]    = g_projMatrix[5] * g_windowHeight * 0.5f;
    wd.viewport[0] = (GLfloat)g_windowWidth;
    wd.viewport[1] = (GLfloat)g_windowHeight;
    wd.camPos[0]   = cx;
    wd.camPos[1]   = cy;
    wd.camPos[2]   = cz;
    wd.camPos[3]   = 1.0f;

    glBindBuffer(GL_UNIFORM_BUFFER, g_worldUBO);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(WorldData), &wd);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    //
    // Depth init compute pass.
    //
    glUseProgram(g_depthProgram.program);
    glUniformMatrix4fv(g_depth_worldMatrixLoc, 1, GL_FALSE, g_worldMatrix);
    glUniform1ui(g_depth_numSplatsLoc, g_numSplats);
    glUniform1ui(g_depth_numSplatsPaddedLoc, g_numSplatsPadded);
    glDispatchCompute(g_numSplatsPadded / SORT_LOCAL_SIZE, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    //
    // Bitonic sort descending by view-space depth.
    //
    glUseProgram(g_sortProgram.program);
    for (k = 2; k <= g_numSplatsPadded; k <<= 1)
    {
        for (j = k >> 1; j >= 1; j >>= 1)
        {
            glUniform1ui(g_sort_jLoc, j);
            glUniform1ui(g_sort_kLoc, k);
            glDispatchCompute(g_numSplatsPadded / SORT_LOCAL_SIZE, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        }
    }

    //
    // Splat render pass.
    //
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_splatProgram.program);
    glBindVertexArray(g_quadVAO);
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)g_numSplats);
    glBindVertexArray(0);
    glUseProgram(0);

    return GLUS_TRUE;
}

// -----------------------------------------------------------------------
// Terminate callback
// -----------------------------------------------------------------------

GLUSvoid terminate(GLUSvoid)
{
    glBindVertexArray(0);

    if (g_quadVAO)
    {
        glDeleteVertexArrays(1, &g_quadVAO);
        g_quadVAO = 0;
    }

    if (g_quadVBO)
    {
        glDeleteBuffers(1, &g_quadVBO);
        g_quadVBO = 0;
    }

    if (g_splatSSBO)
    {
        glDeleteBuffers(1, &g_splatSSBO);
        g_splatSSBO = 0;
    }

    if (g_indexSSBO)
    {
        glDeleteBuffers(1, &g_indexSSBO);
        g_indexSSBO = 0;
    }

    if (g_depthSSBO)
    {
        glDeleteBuffers(1, &g_depthSSBO);
        g_depthSSBO = 0;
    }

    if (g_modelSSBO)
    {
        glDeleteBuffers(1, &g_modelSSBO);
        g_modelSSBO = 0;
    }

    if (g_worldUBO)
    {
        glDeleteBuffers(1, &g_worldUBO);
        g_worldUBO = 0;
    }

    glUseProgram(0);

    glusProgramDestroy(&g_depthProgram);
    glusProgramDestroy(&g_sortProgram);
    glusProgramDestroy(&g_splatProgram);
}

// -----------------------------------------------------------------------
// main
// -----------------------------------------------------------------------

int main(int argc, char* argv[])
{
    EGLint eglConfigAttributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_DEPTH_SIZE, 0,
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

    if (argc >= 2)
    {
        g_gltfPath = argv[1];
    }

    glusLogSetLevel(GLUS_LOG_DEBUG);

    glusWindowSetInitFunc(init);
    glusWindowSetReshapeFunc(reshape);
    glusWindowSetUpdateFunc(update);
    glusWindowSetTerminateFunc(terminate);
    glusWindowSetKeyFunc(key);

    if (!glusWindowCreate("Example 51 - 3D Gaussian Splatting",
                          WINDOW_WIDTH, WINDOW_HEIGHT,
                          GLUS_FALSE, GLUS_FALSE,
                          eglConfigAttributes, eglContextAttributes, 0))
    {
        glusLogPrint(GLUS_LOG_ERROR, "Could not create window.");
        return -1;
    }

    glusWindowRun();

    return 0;
}
