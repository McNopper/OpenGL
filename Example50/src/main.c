/**
 * OpenGL 4 - Example 50
 *
 * @author  Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * glTF 2.0 PBR renderer with Image-Based Lighting and skeletal animation,
 * rendered in Rec.2020 working space and presented through HDR-capable EGL
 * surfaces using McNopper/EGL (https://github.com/McNopper/EGL).
 *
 * Compared to Example49:
 *   1. Working color space is linear Rec.2020 instead of linear Rec.709.
 *      The HDR panorama is converted Rec.709 -> Rec.2020 on the CPU before
 *      IBL pre-filtering, and the PBR fragment shader applies the same
 *      conversion to base-color and emissive texture samples.
 *   2. Window/context is created via McNopper/EGL.  At startup we probe the
 *      EGL extension string and pick the best output color space we find,
 *      preferring BT.2020 (PQ -> HLG -> linear) and falling back through
 *      scRGB, Display P3 and finally sRGB / linear.  The fullscreen
 *      resolve shader applies the matching primaries+EOTF conversion.
 *   3. Camera orbit direction is reversed.
 *
 * Scene loading, mesh upload, the node hierarchy, skinning and TRS animation
 * are handled by the GLUS glTF loader (glusGltfLoadScene /
 * glusGltfUpdateAnimation).
 *
 * Usage:  Example50 [path/to/model.glb]  [path/to/panorama.hdr]
 * Defaults: phoenix/scene.gltf / sunny_rose_garden_4k.hdr next to the binary.
 */

#include <GL/glus.h>

/* GLUS's bundled glus.h defines a small handful of EGL_* enum macros (and an
 * EGLint typedef) for its own use.  Undefine them before pulling in the real
 * McNopper/EGL headers so the macros don't collide with the canonical
 * Khronos definitions (C4005). */
#undef EGL_TRUE
#undef EGL_FALSE
#undef EGL_NONE
#undef EGL_DONT_CARE

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#endif

// ---------------------------------------------------------------------
// Diagnostic logging.  Writes to stdout *and* to Example50.log next to
// the executable; either OutputDebugString or the log file will catch
// the message even when stdout is being discarded by the parent shell.
// ---------------------------------------------------------------------
static FILE* g_logFile = NULL;
static void  log_printf(const char* fmt, ...)
{
    char    buf[4096];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);
    fputs(buf, stdout);
    fflush(stdout);
    if (g_logFile)
    {
        fputs(buf, g_logFile);
        fflush(g_logFile);
    }
#ifdef _WIN32
    OutputDebugStringA(buf);
#endif
}
#define LOGF(...) log_printf(__VA_ARGS__)

// Output colorspace IDs, must match shader/fullscreen.frag.glsl.
enum
{
    OUT_LINEAR            = 0,
    OUT_SRGB              = 1,
    OUT_DISPLAY_P3        = 2,
    OUT_DISPLAY_P3_LINEAR = 3,
    OUT_SCRGB_LINEAR      = 4,
    OUT_SCRGB             = 5,
    OUT_BT2020_LINEAR     = 6,
    OUT_BT2020_HLG        = 7,
    OUT_BT2020_PQ         = 8
};

//
// Constants
//

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define SPECULAR_CUBEMAP_SIZE 256
#define DIFFUSE_CUBEMAP_SIZE 128
#define BRDF_LUT_SIZE 512
#define BACKGROUND_CUBEMAP_SIZE 512
#define NUMBER_ROUGHNESS 6
#define MSAA_SAMPLES 4

#define CAMERA_ORBIT_RADIUS_FACTOR 1.8f
#define CAMERA_ORBIT_RADIUS_MIN 0.5f
#define CAMERA_HEIGHT_OFFSET 0.4f
#define CAMERA_HEIGHT_STEP 0.1f
#define CAMERA_ORBIT_SPEED_STEP 5.0f
#define CAMERA_ZOOM_IN_FACTOR 0.9f
#define CAMERA_ZOOM_OUT_FACTOR 1.1f
#define CAMERA_ZOOM_MIN_FACTOR 0.2f
#define CAMERA_FOV_DEG 45.0f
#define CAMERA_NEAR_FACTOR 0.005f
#define CAMERA_NEAR_MIN 0.001f
#define CAMERA_FAR_RADIUS_FACTOR 3.0f
#define CAMERA_FAR_EXTRA 600.0f
#define SCENE_RADIUS_MIN 0.01f
#define CAMERA_DEG_TO_RAD (GLUS_PI / 180.0f)

//
// Globals
//

// glTF scene (loaded by GLUS).
static GLUSgltfScene g_scene;

// Camera
static GLfloat g_orbitAngle  = 0.0f; // degrees
static GLfloat g_orbitRadius = 5.0f;
static GLfloat g_cameraY     = 0.0f;
static GLfloat g_orbitSpeed  = 18.0f; // deg/s
static GLfloat g_viewProjectionMatrix[16];
static GLfloat g_eye[4];

// Render settings
static GLfloat g_gamma              = 2.2f;
static GLint   g_outputColorspace   = OUT_SRGB; // chosen at startup
static GLfloat g_referenceWhiteNits = 200.0f;   // scene 1.0 -> 200 nits
static GLfloat g_peakNits           = 1000.0f;  // assumed display peak

// IBL textures
static GLuint g_specularTexture  = 0;
static GLuint g_diffuseTexture   = 0;
static GLuint g_brdfLutTexture   = 0;
static GLuint g_bgCubemapTexture = 0;

// Background sphere
static GLuint  g_bgVAO        = 0;
static GLuint  g_bgVBO        = 0;
static GLuint  g_bgIBO        = 0;
static GLsizei g_bgIndexCount = 0;

// Empty VAO for attribute-less fullscreen draw
static GLuint g_fullscreenVAO = 0;

// MSAA framebuffer
static GLuint g_msaaFBO      = 0;
static GLuint g_msaaColor    = 0;
static GLuint g_msaaDepth    = 0;
static GLint  g_windowWidth  = 0;
static GLint  g_windowHeight = 0;

// Programs
static GLUSprogram g_bgProg;
static GLUSprogram g_fullscreenProg;
static GLUSprogram g_pbrProg;
static GLUSprogram g_pbrSkinnedProg;

// Uniform locations - non-skinned PBR
static GLint g_u_modelMatrix;
static GLint g_u_vpMatrix;
static GLint g_u_normalMatrix;
static GLint g_u_eye_pbr;
static GLint g_u_baseColorFactor;
static GLint g_u_metallicFactor;
static GLint g_u_roughnessFactor;
static GLint g_u_emissiveFactor;
static GLint g_u_occlusionStrength;
static GLint g_u_alphaCutoff;
static GLint g_u_alphaMode;
static GLint g_u_hasNormalMap;
static GLint g_u_normalScale;
static GLint g_u_baseColorTexCoordSet;
static GLint g_u_metallicRoughnessTexCoordSet;
static GLint g_u_normalTexCoordSet;
static GLint g_u_occlusionTexCoordSet;
static GLint g_u_emissiveTexCoordSet;

// Uniform locations - skinned PBR
static GLint g_u_vpMatrix_sk;
static GLint g_u_eye_sk;
static GLint g_u_jointMatrices_sk;
static GLint g_u_baseColorFactor_sk;
static GLint g_u_metallicFactor_sk;
static GLint g_u_roughnessFactor_sk;
static GLint g_u_emissiveFactor_sk;
static GLint g_u_occlusionStrength_sk;
static GLint g_u_alphaCutoff_sk;
static GLint g_u_alphaMode_sk;
static GLint g_u_hasNormalMap_sk;
static GLint g_u_normalScale_sk;
static GLint g_u_baseColorTexCoordSet_sk;
static GLint g_u_metallicRoughnessTexCoordSet_sk;
static GLint g_u_normalTexCoordSet_sk;
static GLint g_u_occlusionTexCoordSet_sk;
static GLint g_u_emissiveTexCoordSet_sk;

// Uniform locations - background
static GLint g_u_vpMatrix_bg;

// Uniform locations - fullscreen
static GLint g_u_gamma;
static GLint g_u_msaaSamples;
static GLint g_u_outputColorspace;
static GLint g_u_referenceWhiteNits;
static GLint g_u_peakNits;

// Command-line config
static const GLUSchar* g_gltfPath     = NULL;
static const GLUSchar* g_panoramaPath = NULL;

//
// GLUS callbacks
//

GLUSboolean init(GLUSvoid)
{
    GLUShdrimage panoramaImage;
    GLuint       panoramaTex;
    GLUSshape    sphere;
    GLUStextfile vertexSource;
    GLUStextfile fragmentSource;

    // ----------------------------------------------------------------
    // Build shader programs.
    // ----------------------------------------------------------------

    if (!glusFileLoadText("../Example50/shader/background.vert.glsl", &vertexSource))
    {
        printf("Could not load vertex shader!\n");

        return GLUS_FALSE;
    }

    if (!glusFileLoadText("../Example50/shader/background.frag.glsl", &fragmentSource))
    {
        printf("Could not load fragment shader!\n");

        glusFileDestroyText(&vertexSource);

        return GLUS_FALSE;
    }

    if (!glusProgramBuildFromSource(&g_bgProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
    {
        glusFileDestroyText(&vertexSource);
        glusFileDestroyText(&fragmentSource);
        printf("Failed to build background program\n");
        return GLUS_FALSE;
    }
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    if (!glusFileLoadText("../Example50/shader/fullscreen.vert.glsl", &vertexSource))
    {
        printf("Could not load vertex shader!\n");

        return GLUS_FALSE;
    }

    if (!glusFileLoadText("../Example50/shader/fullscreen.frag.glsl", &fragmentSource))
    {
        printf("Could not load fragment shader!\n");

        glusFileDestroyText(&vertexSource);

        return GLUS_FALSE;
    }

    if (!glusProgramBuildFromSource(&g_fullscreenProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
    {
        glusFileDestroyText(&vertexSource);
        glusFileDestroyText(&fragmentSource);
        printf("Failed to build fullscreen program\n");
        return GLUS_FALSE;
    }
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    if (!glusFileLoadText(GLUS_SHADER_DIR "/glus_gltf_pbr.vert.glsl", &vertexSource))
    {
        printf("Could not load vertex shader!\n");

        return GLUS_FALSE;
    }

    if (!glusFileLoadText(GLUS_SHADER_DIR "/glus_gltf_pbr.frag.glsl", &fragmentSource))
    {
        printf("Could not load fragment shader!\n");

        glusFileDestroyText(&vertexSource);

        return GLUS_FALSE;
    }

    if (!glusProgramBuildFromSource(&g_pbrProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
    {
        glusFileDestroyText(&vertexSource);
        glusFileDestroyText(&fragmentSource);
        printf("Failed to build PBR program\n");
        return GLUS_FALSE;
    }
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    if (!glusFileLoadText(GLUS_SHADER_DIR "/glus_gltf_pbr_skinned.vert.glsl", &vertexSource))
    {
        printf("Could not load vertex shader!\n");

        return GLUS_FALSE;
    }

    if (!glusFileLoadText(GLUS_SHADER_DIR "/glus_gltf_pbr.frag.glsl", &fragmentSource))
    {
        printf("Could not load fragment shader!\n");

        glusFileDestroyText(&vertexSource);

        return GLUS_FALSE;
    }

    if (!glusProgramBuildFromSource(&g_pbrSkinnedProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
    {
        glusFileDestroyText(&vertexSource);
        glusFileDestroyText(&fragmentSource);
        printf("Failed to build skinned PBR program\n");
        return GLUS_FALSE;
    }
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    // ----------------------------------------------------------------
    // Fullscreen VAO (attribute-less draw using gl_VertexID).
    // ----------------------------------------------------------------

    glGenVertexArrays(1, &g_fullscreenVAO);

    // ----------------------------------------------------------------
    // IBL - load panorama and run all four GPU prefilter passes.
    // ----------------------------------------------------------------

    printf("Loading panorama '%s' ...\n", g_panoramaPath);
    if (!glusImageLoadHdr(g_panoramaPath, &panoramaImage))
    {
        printf("Error: failed to load panorama '%s'\n", g_panoramaPath);
        return GLUS_FALSE;
    }

    // Convert the panorama from linear Rec.709 (sRGB primaries) into linear
    // Rec.2020 (BT.2020 primaries) so all derived IBL textures - the
    // pre-filtered specular cubemap, the diffuse irradiance cubemap, and the
    // background cubemap - end up in the working color space already.  The
    // .hdr file is float32 RGB so this conversion is lossless.
    {
        const float m00 = 0.62740389896f, m01 = 0.32928303716f, m02 = 0.04331306388f;
        const float m10 = 0.06909728935f, m11 = 0.91954039507f, m12 = 0.01136231558f;
        const float m20 = 0.01639143887f, m21 = 0.08801330909f, m22 = 0.89559525204f;

        GLfloat* p   = (GLfloat*)panoramaImage.data;
        size_t   pix = (size_t)panoramaImage.width * (size_t)panoramaImage.height;
        for (size_t i = 0; i < pix; ++i)
        {
            float r = p[0], g = p[1], b = p[2];
            p[0] = m00 * r + m01 * g + m02 * b;
            p[1] = m10 * r + m11 * g + m12 * b;
            p[2] = m20 * r + m21 * g + m22 * b;
            p += 3;
        }
    }

    glGenTextures(1, &panoramaTex);
    glBindTexture(GL_TEXTURE_2D, panoramaTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F,
                 panoramaImage.width, panoramaImage.height,
                 0, GL_RGB, GL_FLOAT, panoramaImage.data);
    glusImageDestroyHdr(&panoramaImage);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glusIblSetShaderPath(GLUS_SHADER_DIR "/");

    printf("Pre-filtering specular cubemap (%dx%d, %d levels) ...\n",
           SPECULAR_CUBEMAP_SIZE, SPECULAR_CUBEMAP_SIZE, NUMBER_ROUGHNESS);
    if (!glusIblBuildSpecularEnvironmentMap(&g_specularTexture, panoramaTex,
                                            SPECULAR_CUBEMAP_SIZE, NUMBER_ROUGHNESS))
    {
        printf("Error: glusIblBuildSpecularEnvironmentMap failed\n");
        return GLUS_FALSE;
    }

    printf("Pre-filtering diffuse irradiance cubemap (%dx%d) ...\n",
           DIFFUSE_CUBEMAP_SIZE, DIFFUSE_CUBEMAP_SIZE);
    if (!glusIblBuildDiffuseEnvironmentMap(&g_diffuseTexture, panoramaTex,
                                           DIFFUSE_CUBEMAP_SIZE))
    {
        printf("Error: glusIblBuildDiffuseEnvironmentMap failed\n");
        return GLUS_FALSE;
    }

    printf("Integrating BRDF LUT (%dx%d) ...\n", BRDF_LUT_SIZE, BRDF_LUT_SIZE);
    if (!glusIblBuildBrdfLookupTable(&g_brdfLutTexture, BRDF_LUT_SIZE))
    {
        printf("Error: glusIblBuildBrdfLookupTable failed\n");
        return GLUS_FALSE;
    }

    printf("Building background cubemap (%dx%d) ...\n",
           BACKGROUND_CUBEMAP_SIZE, BACKGROUND_CUBEMAP_SIZE);
    if (!glusIblBuildBackgroundCubemap(&g_bgCubemapTexture, panoramaTex,
                                       BACKGROUND_CUBEMAP_SIZE))
    {
        printf("Error: glusIblBuildBackgroundCubemap failed\n");
        return GLUS_FALSE;
    }

    glDeleteTextures(1, &panoramaTex);

    // ----------------------------------------------------------------
    // Background sphere.
    // ----------------------------------------------------------------

    if (!glusShapeCreateSpheref(&sphere, 500.0f, 32))
    {
        printf("Could not create sphere!\n");

        return GLUS_FALSE;
    }

    g_bgIndexCount = sphere.numberIndices;

    glGenVertexArrays(1, &g_bgVAO);
    glBindVertexArray(g_bgVAO);

    glGenBuffers(1, &g_bgVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_bgVBO);
    glBufferData(GL_ARRAY_BUFFER, sphere.numberVertices * 4 * sizeof(GLfloat),
                 sphere.vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &g_bgIBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_bgIBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere.numberIndices * sizeof(GLuint),
                 sphere.indices, GL_STATIC_DRAW);

    glBindVertexArray(0);
    glusShapeDestroyf(&sphere);

    // ----------------------------------------------------------------
    // Set persistent sampler / scalar uniforms.
    // ----------------------------------------------------------------

    glUseProgram(g_bgProg.program);
    glUniform1i(glGetUniformLocation(g_bgProg.program, "u_texture"), 0);

    glUseProgram(g_fullscreenProg.program);
    glUniform1i(glGetUniformLocation(g_fullscreenProg.program, "u_framebufferTexture"), 0);
    glUniform1i(glGetUniformLocation(g_fullscreenProg.program, "u_msaaSamples"), MSAA_SAMPLES);

    glUseProgram(g_pbrProg.program);
    glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_baseColorTexture"), 0);
    glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_metallicRoughnessTexture"), 1);
    glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_normalTexture"), 2);
    glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_occlusionTexture"), 3);
    glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_emissiveTexture"), 4);
    glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_specularEnvMap"), 5);
    glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_diffuseEnvMap"), 6);
    glUniform1i(glGetUniformLocation(g_pbrProg.program, "u_brdfLUT"), 7);
    glUniform1f(glGetUniformLocation(g_pbrProg.program, "u_roughnessScale"),
                (GLfloat)(NUMBER_ROUGHNESS - 1));

    g_u_modelMatrix       = glGetUniformLocation(g_pbrProg.program, "u_modelMatrix");
    g_u_vpMatrix          = glGetUniformLocation(g_pbrProg.program, "u_viewProjectionMatrix");
    g_u_normalMatrix      = glGetUniformLocation(g_pbrProg.program, "u_normalMatrix");
    g_u_eye_pbr           = glGetUniformLocation(g_pbrProg.program, "u_eye");
    g_u_baseColorFactor   = glGetUniformLocation(g_pbrProg.program, "u_baseColorFactor");
    g_u_metallicFactor    = glGetUniformLocation(g_pbrProg.program, "u_metallicFactor");
    g_u_roughnessFactor   = glGetUniformLocation(g_pbrProg.program, "u_roughnessFactor");
    g_u_emissiveFactor    = glGetUniformLocation(g_pbrProg.program, "u_emissiveFactor");
    g_u_occlusionStrength = glGetUniformLocation(g_pbrProg.program, "u_occlusionStrength");
    g_u_alphaCutoff       = glGetUniformLocation(g_pbrProg.program, "u_alphaCutoff");
    g_u_alphaMode         = glGetUniformLocation(g_pbrProg.program, "u_alphaMode");
    g_u_hasNormalMap      = glGetUniformLocation(g_pbrProg.program, "u_hasNormalMap");
    g_u_normalScale       = glGetUniformLocation(g_pbrProg.program, "u_normalScale");
    g_u_baseColorTexCoordSet         = glGetUniformLocation(g_pbrProg.program, "u_baseColorTexCoordSet");
    g_u_metallicRoughnessTexCoordSet = glGetUniformLocation(g_pbrProg.program, "u_metallicRoughnessTexCoordSet");
    g_u_normalTexCoordSet            = glGetUniformLocation(g_pbrProg.program, "u_normalTexCoordSet");
    g_u_occlusionTexCoordSet         = glGetUniformLocation(g_pbrProg.program, "u_occlusionTexCoordSet");
    g_u_emissiveTexCoordSet          = glGetUniformLocation(g_pbrProg.program, "u_emissiveTexCoordSet");

    glUseProgram(g_pbrSkinnedProg.program);
    glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_baseColorTexture"), 0);
    glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_metallicRoughnessTexture"), 1);
    glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_normalTexture"), 2);
    glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_occlusionTexture"), 3);
    glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_emissiveTexture"), 4);
    glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_specularEnvMap"), 5);
    glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_diffuseEnvMap"), 6);
    glUniform1i(glGetUniformLocation(g_pbrSkinnedProg.program, "u_brdfLUT"), 7);
    glUniform1f(glGetUniformLocation(g_pbrSkinnedProg.program, "u_roughnessScale"),
                (GLfloat)(NUMBER_ROUGHNESS - 1));

    g_u_vpMatrix_sk          = glGetUniformLocation(g_pbrSkinnedProg.program, "u_viewProjectionMatrix");
    g_u_eye_sk               = glGetUniformLocation(g_pbrSkinnedProg.program, "u_eye");
    g_u_jointMatrices_sk     = glGetUniformLocation(g_pbrSkinnedProg.program, "u_jointMatrices");
    g_u_baseColorFactor_sk   = glGetUniformLocation(g_pbrSkinnedProg.program, "u_baseColorFactor");
    g_u_metallicFactor_sk    = glGetUniformLocation(g_pbrSkinnedProg.program, "u_metallicFactor");
    g_u_roughnessFactor_sk   = glGetUniformLocation(g_pbrSkinnedProg.program, "u_roughnessFactor");
    g_u_emissiveFactor_sk    = glGetUniformLocation(g_pbrSkinnedProg.program, "u_emissiveFactor");
    g_u_occlusionStrength_sk = glGetUniformLocation(g_pbrSkinnedProg.program, "u_occlusionStrength");
    g_u_alphaCutoff_sk       = glGetUniformLocation(g_pbrSkinnedProg.program, "u_alphaCutoff");
    g_u_alphaMode_sk         = glGetUniformLocation(g_pbrSkinnedProg.program, "u_alphaMode");
    g_u_hasNormalMap_sk      = glGetUniformLocation(g_pbrSkinnedProg.program, "u_hasNormalMap");
    g_u_normalScale_sk       = glGetUniformLocation(g_pbrSkinnedProg.program, "u_normalScale");
    g_u_baseColorTexCoordSet_sk         = glGetUniformLocation(g_pbrSkinnedProg.program, "u_baseColorTexCoordSet");
    g_u_metallicRoughnessTexCoordSet_sk = glGetUniformLocation(g_pbrSkinnedProg.program, "u_metallicRoughnessTexCoordSet");
    g_u_normalTexCoordSet_sk            = glGetUniformLocation(g_pbrSkinnedProg.program, "u_normalTexCoordSet");
    g_u_occlusionTexCoordSet_sk         = glGetUniformLocation(g_pbrSkinnedProg.program, "u_occlusionTexCoordSet");
    g_u_emissiveTexCoordSet_sk          = glGetUniformLocation(g_pbrSkinnedProg.program, "u_emissiveTexCoordSet");

    g_u_vpMatrix_bg = glGetUniformLocation(g_bgProg.program, "u_viewProjectionMatrix");

    g_u_gamma              = glGetUniformLocation(g_fullscreenProg.program, "u_gamma");
    g_u_msaaSamples        = glGetUniformLocation(g_fullscreenProg.program, "u_msaaSamples");
    g_u_outputColorspace   = glGetUniformLocation(g_fullscreenProg.program, "u_outputColorspace");
    g_u_referenceWhiteNits = glGetUniformLocation(g_fullscreenProg.program, "u_referenceWhiteNits");
    g_u_peakNits           = glGetUniformLocation(g_fullscreenProg.program, "u_peakNits");

    glUseProgram(0);

    // ----------------------------------------------------------------
    // Load glTF scene (GLUS loader).
    // ----------------------------------------------------------------

    printf("Loading glTF: %s\n", g_gltfPath);
    if (!glusGltfLoadScene(g_gltfPath, &g_scene))
    {
        printf("Error: failed to load glTF '%s'\n", g_gltfPath);
        return GLUS_FALSE;
    }

    g_orbitRadius = g_scene.sceneRadius * CAMERA_ORBIT_RADIUS_FACTOR;
    if (g_orbitRadius < CAMERA_ORBIT_RADIUS_MIN)
    {
        g_orbitRadius = CAMERA_ORBIT_RADIUS_MIN;
    }
    g_cameraY = g_scene.sceneCenter[1] + g_scene.sceneRadius * CAMERA_HEIGHT_OFFSET;

    // ----------------------------------------------------------------
    // General GL state.
    // ----------------------------------------------------------------

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    printf("Scene centre (%.2f, %.2f, %.2f)  radius %.2f  orbit %.2f\n",
           g_scene.sceneCenter[0], g_scene.sceneCenter[1], g_scene.sceneCenter[2],
           g_scene.sceneRadius, g_orbitRadius);

    return GLUS_TRUE;
}

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    if (height == 0)
    {
        height = 1;
    }
    g_windowWidth  = width;
    g_windowHeight = height;

    if (g_msaaFBO)
    {
        glDeleteFramebuffers(1, &g_msaaFBO);
        glDeleteTextures(1, &g_msaaColor);
        glDeleteRenderbuffers(1, &g_msaaDepth);
    }

    glGenFramebuffers(1, &g_msaaFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, g_msaaFBO);

    glGenTextures(1, &g_msaaColor);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, g_msaaColor);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, MSAA_SAMPLES,
                            GL_RGBA16F, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D_MULTISAMPLE, g_msaaColor, 0);

    glGenRenderbuffers(1, &g_msaaDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, g_msaaDepth);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, MSAA_SAMPLES,
                                     GL_DEPTH_COMPONENT24, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                              GL_RENDERBUFFER, g_msaaDepth);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("GL_FRAMEBUFFER_COMPLETE error 0x%x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Bind material textures and select the correct PBR program.
static void bindPrimitiveMaterial(const GLUSgltfPrimitive* gp)
{
    const GLUSgltfMaterial* m = &gp->material;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m->baseColorTexture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, m->metallicRoughnessTexture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, m->normalTexture);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, m->occlusionTexture);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, m->emissiveTexture);

    if (gp->skinIndex >= 0 && gp->skinIndex < g_scene.skinCount)
    {
        const GLUSgltfSkin* gs = &g_scene.skins[gp->skinIndex];
        glUseProgram(g_pbrSkinnedProg.program);
        glUniformMatrix4fv(g_u_jointMatrices_sk, gs->jointCount, GL_FALSE, gs->jointMatrices);
        glUniform4fv(g_u_baseColorFactor_sk, 1, m->baseColorFactor);
        glUniform1f(g_u_metallicFactor_sk, m->metallicFactor);
        glUniform1f(g_u_roughnessFactor_sk, m->roughnessFactor);
        glUniform3fv(g_u_emissiveFactor_sk, 1, m->emissiveFactor);
        glUniform1f(g_u_occlusionStrength_sk, m->occlusionStrength);
        glUniform1f(g_u_alphaCutoff_sk, m->alphaCutoff);
        glUniform1i(g_u_alphaMode_sk, m->alphaMode);
        glUniform1i(g_u_hasNormalMap_sk, m->hasNormalMap);
        glUniform1f(g_u_normalScale_sk, m->normalScale);
        glUniform1i(g_u_baseColorTexCoordSet_sk, m->baseColorTexCoordSet);
        glUniform1i(g_u_metallicRoughnessTexCoordSet_sk, m->metallicRoughnessTexCoordSet);
        glUniform1i(g_u_normalTexCoordSet_sk, m->normalTexCoordSet);
        glUniform1i(g_u_occlusionTexCoordSet_sk, m->occlusionTexCoordSet);
        glUniform1i(g_u_emissiveTexCoordSet_sk, m->emissiveTexCoordSet);
    }
    else
    {
        glUseProgram(g_pbrProg.program);
        glUniformMatrix4fv(g_u_modelMatrix, 1, GL_FALSE, gp->modelMatrix);
        glUniformMatrix3fv(g_u_normalMatrix, 1, GL_FALSE, gp->normalMatrix);
        glUniform4fv(g_u_baseColorFactor, 1, m->baseColorFactor);
        glUniform1f(g_u_metallicFactor, m->metallicFactor);
        glUniform1f(g_u_roughnessFactor, m->roughnessFactor);
        glUniform3fv(g_u_emissiveFactor, 1, m->emissiveFactor);
        glUniform1f(g_u_occlusionStrength, m->occlusionStrength);
        glUniform1f(g_u_alphaCutoff, m->alphaCutoff);
        glUniform1i(g_u_alphaMode, m->alphaMode);
        glUniform1i(g_u_hasNormalMap, m->hasNormalMap);
        glUniform1f(g_u_normalScale, m->normalScale);
        glUniform1i(g_u_baseColorTexCoordSet, m->baseColorTexCoordSet);
        glUniform1i(g_u_metallicRoughnessTexCoordSet, m->metallicRoughnessTexCoordSet);
        glUniform1i(g_u_normalTexCoordSet, m->normalTexCoordSet);
        glUniform1i(g_u_occlusionTexCoordSet, m->occlusionTexCoordSet);
        glUniform1i(g_u_emissiveTexCoordSet, m->emissiveTexCoordSet);
    }
}

GLUSboolean update(GLUSfloat time)
{
    GLfloat rad;
    GLfloat eyeX, eyeY, eyeZ;
    GLfloat nearPlane, farPlane;
    GLfloat viewMatrix[16];
    GLfloat projMatrix[16];
    GLfloat bgViewMatrix[16];
    GLfloat bgVPMatrix[16];
    GLint   i;

    if (!g_msaaFBO)
    {
        return GLUS_TRUE;
    }

    // --- Advance skinning / TRS animation (GLUS) ---
    glusGltfUpdateAnimation(&g_scene, time);

    // --- Orbit camera ---
    // Reverse-orbit camera (opposite direction to Example49).
    g_orbitAngle += g_orbitSpeed * time;
    if (g_orbitAngle >= 360.0f)
    {
        g_orbitAngle = fmodf(g_orbitAngle, 360.0f);
    }

    rad  = g_orbitAngle * CAMERA_DEG_TO_RAD;
    eyeX = g_scene.sceneCenter[0] + g_orbitRadius * sinf(rad);
    eyeY = g_cameraY;
    eyeZ = g_scene.sceneCenter[2] + g_orbitRadius * cosf(rad);

    g_eye[0] = eyeX;
    g_eye[1] = eyeY;
    g_eye[2] = eyeZ;
    g_eye[3] = 1.0f;

    nearPlane = g_scene.sceneRadius * CAMERA_NEAR_FACTOR;
    farPlane  = g_orbitRadius + g_scene.sceneRadius * CAMERA_FAR_RADIUS_FACTOR + CAMERA_FAR_EXTRA;
    if (nearPlane < CAMERA_NEAR_MIN)
    {
        nearPlane = CAMERA_NEAR_MIN;
    }

    glusMatrix4x4LookAtf(viewMatrix, eyeX, eyeY, eyeZ,
                         g_scene.sceneCenter[0], g_scene.sceneCenter[1], g_scene.sceneCenter[2],
                         0.0f, 1.0f, 0.0f);
    glusMatrix4x4Perspectivef(projMatrix, CAMERA_FOV_DEG,
                              (GLfloat)g_windowWidth / (GLfloat)g_windowHeight,
                              nearPlane, farPlane);
    glusMatrix4x4Multiplyf(g_viewProjectionMatrix, projMatrix, viewMatrix);

    memcpy(bgViewMatrix, viewMatrix, 16 * sizeof(GLfloat));
    bgViewMatrix[12] = 0.0f;
    bgViewMatrix[13] = 0.0f;
    bgViewMatrix[14] = 0.0f;
    glusMatrix4x4Multiplyf(bgVPMatrix, projMatrix, bgViewMatrix);

    // --- Render into MSAA FBO ---
    glBindFramebuffer(GL_FRAMEBUFFER, g_msaaFBO);
    glViewport(0, 0, g_windowWidth, g_windowHeight);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // --- Background sphere ---
    glFrontFace(GL_CW);
    glDisable(GL_CULL_FACE);
    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glUseProgram(g_bgProg.program);
    glUniformMatrix4fv(g_u_vpMatrix_bg, 1, GL_FALSE, bgVPMatrix);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, g_bgCubemapTexture);
    glBindVertexArray(g_bgVAO);
    glDrawElements(GL_TRIANGLES, g_bgIndexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glFrontFace(GL_CCW);
    glDepthMask(GL_TRUE);

    // --- IBL textures on units 5-7 (persistent per frame) ---
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, g_specularTexture);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, g_diffuseTexture);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, g_brdfLutTexture);

    // Set VP + eye on both PBR programs before the draw loop.
    glUseProgram(g_pbrProg.program);
    glUniformMatrix4fv(g_u_vpMatrix, 1, GL_FALSE, g_viewProjectionMatrix);
    glUniform4fv(g_u_eye_pbr, 1, g_eye);

    glUseProgram(g_pbrSkinnedProg.program);
    glUniformMatrix4fv(g_u_vpMatrix_sk, 1, GL_FALSE, g_viewProjectionMatrix);
    glUniform4fv(g_u_eye_sk, 1, g_eye);

    // --- Pass A: Opaque + Mask ---
    glEnable(GL_DEPTH_TEST);
    for (i = 0; i < g_scene.primitiveCount; i++)
    {
        const GLUSgltfPrimitive* gp = &g_scene.primitives[i];
        if (gp->material.alphaMode == GLUS_GLTF_ALPHA_BLEND)
        {
            continue;
        }
        if (gp->material.doubleSided)
        {
            glDisable(GL_CULL_FACE);
        }
        else
        {
            glEnable(GL_CULL_FACE);
        }
        bindPrimitiveMaterial(gp);
        glusGltfDrawPrimitive(&g_scene, i);
    }

    // --- Pass B: Blend ---
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);

    for (i = 0; i < g_scene.primitiveCount; i++)
    {
        const GLUSgltfPrimitive* gp = &g_scene.primitives[i];
        if (gp->material.alphaMode != GLUS_GLTF_ALPHA_BLEND)
        {
            continue;
        }
        if (gp->material.doubleSided)
        {
            glDisable(GL_CULL_FACE);
        }
        else
        {
            glEnable(GL_CULL_FACE);
        }
        bindPrimitiveMaterial(gp);
        glusGltfDrawPrimitive(&g_scene, i);
    }

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);

    // --- Fullscreen resolve: MSAA -> default FBO with tone-map ---
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, g_windowWidth, g_windowHeight);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_MULTISAMPLE);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(g_fullscreenProg.program);
    glUniform1f(g_u_gamma, g_gamma);
    glUniform1i(g_u_msaaSamples, MSAA_SAMPLES);
    glUniform1i(g_u_outputColorspace, g_outputColorspace);
    glUniform1f(g_u_referenceWhiteNits, g_referenceWhiteNits);
    glUniform1f(g_u_peakNits, g_peakNits);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, g_msaaColor);
    glBindVertexArray(g_fullscreenVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    glEnable(GL_DEPTH_TEST);

    return GLUS_TRUE;
}

GLUSvoid terminate(GLUSvoid)
{
    glusGltfDestroyScene(&g_scene);

    glDeleteTextures(1, &g_specularTexture);
    glDeleteTextures(1, &g_diffuseTexture);
    glDeleteTextures(1, &g_brdfLutTexture);
    glDeleteTextures(1, &g_bgCubemapTexture);

    glDeleteVertexArrays(1, &g_bgVAO);
    glDeleteBuffers(1, &g_bgVBO);
    glDeleteBuffers(1, &g_bgIBO);

    glDeleteVertexArrays(1, &g_fullscreenVAO);

    if (g_msaaFBO)
    {
        glDeleteFramebuffers(1, &g_msaaFBO);
        glDeleteTextures(1, &g_msaaColor);
        glDeleteRenderbuffers(1, &g_msaaDepth);
    }

    glusProgramDestroy(&g_bgProg);
    glusProgramDestroy(&g_fullscreenProg);
    glusProgramDestroy(&g_pbrProg);
    glusProgramDestroy(&g_pbrSkinnedProg);

    if (g_logFile)
    {
        fclose(g_logFile);

        g_logFile = NULL;
    }
}

GLUSvoid key(const GLUSboolean pressed, const GLUSint key)
{
    if (!pressed)
    {
        return;
    }

    switch (key)
    {
    case 265:
        g_cameraY += g_scene.sceneRadius * CAMERA_HEIGHT_STEP;
        break;
    case 264:
        g_cameraY -= g_scene.sceneRadius * CAMERA_HEIGHT_STEP;
        break;
    case 262:
        g_orbitSpeed += CAMERA_ORBIT_SPEED_STEP;
        break;
    case 263:
        g_orbitSpeed -= CAMERA_ORBIT_SPEED_STEP;
        if (g_orbitSpeed < 0.0f)
        {
            g_orbitSpeed = 0.0f;
        }
        break;
    case '+':
    case '=':
        g_orbitRadius *= CAMERA_ZOOM_IN_FACTOR;
        if (g_orbitRadius < g_scene.sceneRadius * CAMERA_ZOOM_MIN_FACTOR)
        {
            g_orbitRadius = g_scene.sceneRadius * CAMERA_ZOOM_MIN_FACTOR;
        }
        break;
    case '-':
        g_orbitRadius *= CAMERA_ZOOM_OUT_FACTOR;
        break;
    default:
        break;
    }
}

// ---------------------------------------------------------------------
// Output color space selection and Win32 + McNopper/EGL bootstrap.
// ---------------------------------------------------------------------

typedef struct
{
    int         id;         // OUT_*
    const char* name;       // human-readable
    const char* extension;  // EGL extension that signals support
    EGLint      colorspace; // EGL_GL_COLORSPACE_* attribute value
    EGLint      redSize;
    EGLint      greenSize;
    EGLint      blueSize;
    EGLint      alphaSize;
} OutputColorspaceCandidate;

// Candidates ordered by preference.  The user explicitly asked for Rec.2020
// preferred, so all BT.2020 variants come first; scRGB / Display P3 / sRGB
// are progressive fallbacks.
static const OutputColorspaceCandidate g_candidates[] = {
    {OUT_BT2020_PQ, "BT.2020 PQ (HDR10)", "EGL_EXT_gl_colorspace_bt2020_pq", EGL_GL_COLORSPACE_BT2020_PQ_EXT, 10, 10, 10, 2},
    {OUT_BT2020_HLG, "BT.2020 HLG", "EGL_EXT_gl_colorspace_bt2020_hlg", EGL_GL_COLORSPACE_BT2020_HLG_EXT, 10, 10, 10, 2},
    {OUT_BT2020_LINEAR, "BT.2020 linear", "EGL_EXT_gl_colorspace_bt2020_linear", EGL_GL_COLORSPACE_BT2020_LINEAR_EXT, 16, 16, 16, 16},
    {OUT_SCRGB_LINEAR, "scRGB linear (fp16)", "EGL_EXT_gl_colorspace_scrgb_linear", EGL_GL_COLORSPACE_SCRGB_LINEAR_EXT, 16, 16, 16, 16},
    {OUT_SCRGB, "scRGB", "EGL_EXT_gl_colorspace_scrgb", EGL_GL_COLORSPACE_SCRGB_EXT, 16, 16, 16, 16},
    {OUT_DISPLAY_P3_LINEAR, "Display P3 linear", "EGL_EXT_gl_colorspace_display_p3_linear", EGL_GL_COLORSPACE_DISPLAY_P3_LINEAR_EXT, 16, 16, 16, 16},
    {OUT_DISPLAY_P3, "Display P3", "EGL_EXT_gl_colorspace_display_p3", EGL_GL_COLORSPACE_DISPLAY_P3_EXT, 8, 8, 8, 8},
    {OUT_SRGB, "sRGB", NULL, EGL_GL_COLORSPACE_SRGB, 8, 8, 8, 8},
    {OUT_LINEAR, "linear (Rec.709)", NULL, EGL_GL_COLORSPACE_LINEAR, 8, 8, 8, 8},
};
static const int g_numCandidates = (int)(sizeof(g_candidates) / sizeof(g_candidates[0]));

#ifdef _WIN32

static const wchar_t* WIN32_CLASS_NAME = L"Example50WindowClass";

static HWND   g_hwnd            = NULL;
static HDC    g_hdc             = NULL;
static GLint  g_runWidth        = SCREEN_WIDTH;
static GLint  g_runHeight       = SCREEN_HEIGHT;
static int    g_shouldClose     = 0;
static int    g_glReady         = 0;
static double g_lastTimeSeconds = 0.0;

static double winNowSeconds(void)
{
    LARGE_INTEGER c, f;
    QueryPerformanceCounter(&c);
    QueryPerformanceFrequency(&f);
    return (double)c.QuadPart / (double)f.QuadPart;
}

static int translateGlfwKey(WPARAM wp)
{
    // Reuse the GLFW key codes that Example49's key() handler already speaks.
    switch (wp)
    {
    case VK_UP: return 265;
    case VK_DOWN: return 264;
    case VK_LEFT: return 263;
    case VK_RIGHT: return 262;
    case VK_OEM_PLUS:
    case VK_ADD: return '+';
    case VK_OEM_MINUS:
    case VK_SUBTRACT: return '-';
    default:
        if (wp >= 0x20 && wp < 0x80)
        {
            return (int)wp;
        }
        return -1;
    }
}

static LRESULT CALLBACK winProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp)
{
    switch (msg)
    {
    case WM_CLOSE:
    case WM_DESTROY:
        g_shouldClose = 1;
        PostQuitMessage(0);
        return 0;
    case WM_SIZE:
        g_runWidth  = LOWORD(lp);
        g_runHeight = HIWORD(lp);
        if (g_glReady && g_runWidth > 0 && g_runHeight > 0)
        {
            reshape(g_runWidth, g_runHeight);
        }
        return 0;
    case WM_KEYDOWN:
        if (wp == VK_ESCAPE)
        {
            g_shouldClose = 1;
            PostQuitMessage(0);
            return 0;
        }
        {
            int k = translateGlfwKey(wp);
            if (k >= 0)
            {
                key(GLUS_TRUE, k);
            }
        }
        return 0;
    case WM_KEYUP:
    {
        int k = translateGlfwKey(wp);
        if (k >= 0)
        {
            key(GLUS_FALSE, k);
        }
    }
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

static HWND createNativeWindow(int width, int height, const char* title)
{
    HINSTANCE hi = GetModuleHandleW(NULL);
    WNDCLASSW wc;
    memset(&wc, 0, sizeof(wc));
    wc.style         = CS_OWNDC;
    wc.lpfnWndProc   = winProc;
    wc.hInstance     = hi;
    wc.hCursor       = LoadCursorW(NULL, (LPCWSTR)IDC_ARROW);
    wc.lpszClassName = WIN32_CLASS_NAME;
    RegisterClassW(&wc);

    wchar_t wtitle[256];
    MultiByteToWideChar(CP_UTF8, 0, title, -1, wtitle, 256);

    RECT r = {0, 0, width, height};
    AdjustWindowRect(&r, WS_OVERLAPPEDWINDOW, FALSE);

    HWND hwnd = CreateWindowExW(0, WIN32_CLASS_NAME, wtitle,
                                WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT,
                                r.right - r.left, r.bottom - r.top,
                                NULL, NULL, hi, NULL);
    return hwnd;
}

#endif // _WIN32

static int extensionPresent(const char* exts, const char* ext)
{
    if (!exts || !ext)
    {
        return 0;
    }
    const char* p = strstr(exts, ext);
    if (!p)
    {
        return 0;
    }
    char tail = p[strlen(ext)];
    return (tail == ' ' || tail == '\0');
}

int main(int argc, char** argv)
{
#ifndef _WIN32
    (void)argc;
    (void)argv;
    fprintf(stderr, "Example50 currently only ships a Windows native-window backend.\n");
    return 1;
#else
    g_gltfPath     = (argc > 1) ? argv[1] : "phoenix/scene.gltf";
    g_panoramaPath = (argc > 2) ? argv[2] : "sunny_rose_garden_4k.hdr";

    // Force every printf to flush immediately and also mirror to a log
    // file next to the executable so we can diagnose runs that finish
    // (or hang) before any pipe-buffered output is visible.
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);
    g_logFile = fopen("Example50.log", "w");
    if (g_logFile)
    {
        setvbuf(g_logFile, NULL, _IONBF, 0);
    }
    LOGF("Example50 starting...\n");
    LOGF("  glTF      : %s\n", g_gltfPath);
    LOGF("  panorama  : %s\n", g_panoramaPath);

    if (!glewExperimental)
    {
        glewExperimental = GL_TRUE;
    }

    // Opt into per-monitor DPI v2 awareness so the window we ask Windows for
    // (SCREEN_WIDTH x SCREEN_HEIGHT logical pixels) is created at that exact
    // physical-pixel size on high-DPI displays, instead of being DPI-virtualised
    // (which would silently inflate the EGL/Vulkan swapchain to the monitor's
    // physical resolution - e.g. 3840x2160 on a 4K display at 300% scaling).
    // Resolved dynamically so the binary still loads on older Windows versions.
    {
        HMODULE user32 = GetModuleHandleW(L"user32.dll");
        if (user32)
        {
            typedef BOOL(WINAPI * PFN_SetProcessDpiAwarenessContext)(HANDLE);
            PFN_SetProcessDpiAwarenessContext pSetCtx =
                (PFN_SetProcessDpiAwarenessContext)GetProcAddress(user32, "SetProcessDpiAwarenessContext");
            if (pSetCtx)
            {
                /* DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = ((HANDLE)-4) */
                if (!pSetCtx((HANDLE)-4))
                {
                    /* Fall back to v1 per-monitor awareness if v2 isn't available. */
                    pSetCtx((HANDLE)-3);
                }
            }
            else
            {
                /* Pre-1703 Windows 10: best-effort, system-DPI awareness only. */
                typedef BOOL(WINAPI * PFN_SetProcessDPIAware)(void);
                PFN_SetProcessDPIAware pAware =
                    (PFN_SetProcessDPIAware)GetProcAddress(user32, "SetProcessDPIAware");
                if (pAware)
                {
                    pAware();
                }
            }
        }
    }

    g_hwnd = createNativeWindow(SCREEN_WIDTH, SCREEN_HEIGHT,
                                "Example 50 - Rec.2020 PBR + IBL + HDR EGL");
    if (!g_hwnd)
    {
        LOGF("createNativeWindow failed (GetLastError=%lu)\n", GetLastError());
        return 1;
    }
    g_hdc = GetDC(g_hwnd);
    LOGF("Native window created: HWND=%p HDC=%p\n", (void*)g_hwnd, (void*)g_hdc);

    // Initialize EGL using McNopper/EGL.  Their own examples use
    // EGL_DEFAULT_DISPLAY, so try that first; HDC is a documented but
    // less-tested path in the implementation.
    EGLDisplay dpy = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (dpy == EGL_NO_DISPLAY)
    {
        dpy = eglGetDisplay((EGLNativeDisplayType)g_hdc);
    }
    if (dpy == EGL_NO_DISPLAY)
    {
        LOGF("eglGetDisplay failed: 0x%x\n", eglGetError());
        return 1;
    }
    LOGF("eglGetDisplay -> %p\n", (void*)dpy);

    EGLint eglMajor = 0, eglMinor = 0;
    if (!eglInitialize(dpy, &eglMajor, &eglMinor))
    {
        LOGF("eglInitialize failed: 0x%x\n", eglGetError());
        return 1;
    }
    LOGF("EGL %d.%d initialized via McNopper/EGL\n", eglMajor, eglMinor);
    LOGF("EGL_VENDOR     : %s\n", eglQueryString(dpy, EGL_VENDOR));
    LOGF("EGL_VERSION    : %s\n", eglQueryString(dpy, EGL_VERSION));
    LOGF("EGL_CLIENT_APIS: %s\n", eglQueryString(dpy, EGL_CLIENT_APIS));

    const char* eglExts = eglQueryString(dpy, EGL_EXTENSIONS);
    LOGF("EGL_EXTENSIONS : %s\n", eglExts ? eglExts : "(null)");

    // Per-extension diagnostic: list which colorspace EXT we recognise.
    for (int i = 0; i < g_numCandidates; ++i)
    {
        const OutputColorspaceCandidate* c = &g_candidates[i];
        if (c->extension == NULL)
        {
            continue;
        }
        LOGF("  %-44s : %s\n", c->extension,
             extensionPresent(eglExts, c->extension) ? "supported" : "absent");
    }

    if (!eglBindAPI(EGL_OPENGL_API))
    {
        LOGF("eglBindAPI(EGL_OPENGL_API) failed: 0x%x\n", eglGetError());
        return 1;
    }

    // Pick the best supported output color space (informational only;
    // the surface-creation loop below confirms what actually works).
    const OutputColorspaceCandidate* chosen = NULL;
    for (int i = 0; i < g_numCandidates; ++i)
    {
        const OutputColorspaceCandidate* c = &g_candidates[i];
        if (c->extension == NULL || extensionPresent(eglExts, c->extension))
        {
            chosen = c;
            break;
        }
    }
    if (!chosen)
    {
        chosen = &g_candidates[g_numCandidates - 1];
    }
    g_outputColorspace = chosen->id;
    LOGF("Preferred output colorspace: %s\n", chosen->name);

    // Build EGL config + surface for the chosen color space.  If anything
    // fails (e.g. driver advertises the extension but the surface format
    // still won't validate) walk down the candidate list.
    EGLConfig  cfg     = NULL;
    EGLSurface surface = EGL_NO_SURFACE;
    for (int i = 0; i < g_numCandidates && surface == EGL_NO_SURFACE; ++i)
    {
        const OutputColorspaceCandidate* c = &g_candidates[i];
        if (c->extension && !extensionPresent(eglExts, c->extension))
        {
            LOGF("  [%-22s] skipped (extension absent)\n", c->name);
            continue;
        }

        EGLint cfgAttribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
            EGL_RED_SIZE, c->redSize,
            EGL_GREEN_SIZE, c->greenSize,
            EGL_BLUE_SIZE, c->blueSize,
            EGL_ALPHA_SIZE, c->alphaSize,
            EGL_DEPTH_SIZE, 24,
            EGL_NONE};
        EGLint    numCfg = 0;
        EGLConfig tryCfg = NULL;
        if (!eglChooseConfig(dpy, cfgAttribs, &tryCfg, 1, &numCfg) || numCfg == 0)
        {
            LOGF("  [%-22s] eglChooseConfig found 0 (err=0x%x)\n", c->name, eglGetError());
            continue;
        }

        EGLint surfAttribs[] = {
            EGL_GL_COLORSPACE, c->colorspace,
            EGL_NONE};
        EGLSurface trySurf = eglCreateWindowSurface(dpy, tryCfg,
                                                    (EGLNativeWindowType)g_hwnd, surfAttribs);
        if (trySurf == EGL_NO_SURFACE)
        {
            LOGF("  [%-22s] eglCreateWindowSurface failed (err=0x%x)\n", c->name, eglGetError());
            continue;
        }

        cfg                = tryCfg;
        surface            = trySurf;
        chosen             = c;
        g_outputColorspace = c->id;
        LOGF("  [%-22s] surface created OK <-- using this\n", c->name);
    }
    if (surface == EGL_NO_SURFACE)
    {
        LOGF("Could not create any EGL window surface (last error 0x%x)\n", eglGetError());
        return 1;
    }

    // HDR10 mastering metadata (only meaningful for BT.2020 PQ; harmless
    // otherwise - eglSurfaceAttrib will silently ignore unsupported attribs).
    if (g_outputColorspace == OUT_BT2020_PQ)
    {
        const struct
        {
            EGLint a;
            EGLint v;
        } md[] = {
            {EGL_SMPTE2086_DISPLAY_PRIMARY_RX_EXT, (EGLint)(0.708f * 50000)},
            {EGL_SMPTE2086_DISPLAY_PRIMARY_RY_EXT, (EGLint)(0.292f * 50000)},
            {EGL_SMPTE2086_DISPLAY_PRIMARY_GX_EXT, (EGLint)(0.170f * 50000)},
            {EGL_SMPTE2086_DISPLAY_PRIMARY_GY_EXT, (EGLint)(0.797f * 50000)},
            {EGL_SMPTE2086_DISPLAY_PRIMARY_BX_EXT, (EGLint)(0.131f * 50000)},
            {EGL_SMPTE2086_DISPLAY_PRIMARY_BY_EXT, (EGLint)(0.046f * 50000)},
            {EGL_SMPTE2086_WHITE_POINT_X_EXT, (EGLint)(0.3127f * 50000)},
            {EGL_SMPTE2086_WHITE_POINT_Y_EXT, (EGLint)(0.3290f * 50000)},
            {EGL_SMPTE2086_MAX_LUMINANCE_EXT, (EGLint)(g_peakNits * 10000.0f)},
            {EGL_SMPTE2086_MIN_LUMINANCE_EXT, (EGLint)(0.001f * 10000)},
        };
        for (size_t i = 0; i < sizeof(md) / sizeof(md[0]); ++i)
        {
            eglSurfaceAttrib(dpy, surface, md[i].a, md[i].v);
        }
    }

    const EGLint ctxAttribs[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 6,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
        EGL_NONE};
    EGLContext ctx = eglCreateContext(dpy, cfg, EGL_NO_CONTEXT, ctxAttribs);
    if (ctx == EGL_NO_CONTEXT)
    {
        LOGF("eglCreateContext failed: 0x%x\n", eglGetError());
        return 1;
    }
    if (!eglMakeCurrent(dpy, surface, surface, ctx))
    {
        LOGF("eglMakeCurrent failed: 0x%x\n", eglGetError());
        return 1;
    }
    LOGF("EGL context current.\n");

    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK)
    {
        LOGF("glewInit failed: %s\n", glewGetErrorString(glewErr));
        return 1;
    }
    LOGF("GL_VENDOR  : %s\n", (const char*)glGetString(GL_VENDOR));
    LOGF("GL_RENDERER: %s\n", (const char*)glGetString(GL_RENDERER));
    LOGF("GL_VERSION : %s\n", (const char*)glGetString(GL_VERSION));

    // Make the window visible only after we have a working context, so the
    // user doesn't see a black flash if surface creation fails.
    ShowWindow(g_hwnd, SW_SHOW);
    UpdateWindow(g_hwnd);

    g_runWidth  = SCREEN_WIDTH;
    g_runHeight = SCREEN_HEIGHT;
    LOGF("Calling init() ...\n");
    if (!init())
    {
        LOGF("init() failed\n");
        return 1;
    }
    LOGF("init() OK; entering main loop.\n");
    g_glReady = 1;
    reshape(g_runWidth, g_runHeight);

    g_lastTimeSeconds = winNowSeconds();

    while (!g_shouldClose)
    {
        MSG msg;
        while (PeekMessageW(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (g_shouldClose)
        {
            break;
        }

        double now        = winNowSeconds();
        float  dt         = (float)(now - g_lastTimeSeconds);
        g_lastTimeSeconds = now;

        if (!update(dt))
        {
            break;
        }
        eglSwapBuffers(dpy, surface);
    }

    terminate();
    eglMakeCurrent(dpy, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    eglDestroyContext(dpy, ctx);
    eglDestroySurface(dpy, surface);
    eglTerminate(dpy);

    if (g_hdc)
    {
        ReleaseDC(g_hwnd, g_hdc);
    }
    DestroyWindow(g_hwnd);
    UnregisterClassW(WIN32_CLASS_NAME, GetModuleHandleW(NULL));

    return 0;
#endif
}
