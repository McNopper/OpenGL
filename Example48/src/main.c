/**
 * OpenGL 4 - Example 48
 *
 * @author  Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * glTF 2.0 PBR renderer with Image-Based Lighting.
 *
 * Scene loading, mesh upload and the node hierarchy are handled by the GLUS
 * glTF loader (glusGltfLoadScene). This example owns the IBL pre-filtering, the
 * camera, the MSAA resolve pass and the per-primitive material binding.
 *
 * Usage:  Example48 [path/to/model.glb]  [path/to/panorama.hdr]
 * Defaults: einstein/scene.gltf / sunny_rose_garden_4k.hdr next to the binary.
 */

#include <GL/glus.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
static GLfloat g_gamma = 2.2f;

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

// Uniform locations - PBR
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

// Uniform locations - background
static GLint g_u_vpMatrix_bg;

// Uniform locations - fullscreen
static GLint g_u_gamma;
static GLint g_u_msaaSamples;

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
    GLfloat      dx, dy, dz;

    // ----------------------------------------------------------------
    // Build shader programs for background and fullscreen resolve.
    // The PBR and IBL shaders are loaded from the GLUS shader directory.
    // ----------------------------------------------------------------

    glusFileLoadText("../Example48/shader/background.vert.glsl", &vertexSource);
    glusFileLoadText("../Example48/shader/background.frag.glsl", &fragmentSource);
    if (!glusProgramBuildFromSource(&g_bgProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
    {
        glusFileDestroyText(&vertexSource);
        glusFileDestroyText(&fragmentSource);
        printf("Failed to build background program\n");
        return GLUS_FALSE;
    }
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    glusFileLoadText("../Example48/shader/fullscreen.vert.glsl", &vertexSource);
    glusFileLoadText("../Example48/shader/fullscreen.frag.glsl", &fragmentSource);
    if (!glusProgramBuildFromSource(&g_fullscreenProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
    {
        glusFileDestroyText(&vertexSource);
        glusFileDestroyText(&fragmentSource);
        printf("Failed to build fullscreen program\n");
        return GLUS_FALSE;
    }
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    glusFileLoadText(GLUS_SHADER_DIR "/glus_gltf_pbr.vert.glsl", &vertexSource);
    glusFileLoadText(GLUS_SHADER_DIR "/glus_gltf_pbr.frag.glsl", &fragmentSource);
    if (!glusProgramBuildFromSource(&g_pbrProg, (const GLUSchar**)&vertexSource.text, 0, 0, 0, (const GLUSchar**)&fragmentSource.text))
    {
        glusFileDestroyText(&vertexSource);
        glusFileDestroyText(&fragmentSource);
        printf("Failed to build PBR program\n");
        return GLUS_FALSE;
    }
    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    // ----------------------------------------------------------------
    // Fullscreen VAO (attribute-less draw using gl_VertexID).
    // ----------------------------------------------------------------

    glGenVertexArrays(1, &g_fullscreenVAO);

    // ----------------------------------------------------------------
    // IBL - load panorama and run all four GPU prefilter passes via
    // the GLUS IBL API.  Panorama texture is deleted afterwards.
    // ----------------------------------------------------------------

    printf("Loading panorama '%s' ...\n", g_panoramaPath);
    if (!glusImageLoadHdr(g_panoramaPath, &panoramaImage))
    {
        printf("Error: failed to load panorama '%s'\n", g_panoramaPath);
        return GLUS_FALSE;
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

    glusShapeCreateSpheref(&sphere, 500.0f, 32);
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

    g_u_vpMatrix_bg = glGetUniformLocation(g_bgProg.program, "u_viewProjectionMatrix");

    g_u_gamma       = glGetUniformLocation(g_fullscreenProg.program, "u_gamma");
    g_u_msaaSamples = glGetUniformLocation(g_fullscreenProg.program, "u_msaaSamples");

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

    (void)dx;
    (void)dy;
    (void)dz;
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

// Called when the window is resized.
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

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

// Helper: bind all material textures for a primitive and set uniforms.
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

    // --- Orbit camera ---
    g_orbitAngle += g_orbitSpeed * time;
    if (g_orbitAngle >= 360.0f)
    {
        g_orbitAngle -= 360.0f;
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

    // --- Background sphere (inside-facing, no depth write) ---
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

    // --- PBR IBL textures on units 5-7 (persistent per frame) ---
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, g_specularTexture);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_CUBE_MAP, g_diffuseTexture);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, g_brdfLutTexture);

    glUseProgram(g_pbrProg.program);
    glUniformMatrix4fv(g_u_vpMatrix, 1, GL_FALSE, g_viewProjectionMatrix);
    glUniform4fv(g_u_eye_pbr, 1, g_eye);

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

int main(int argc, char** argv)
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

    g_gltfPath     = (argc > 1) ? argv[1] : "einstein/scene.gltf";
    g_panoramaPath = (argc > 2) ? argv[2] : "sunny_rose_garden_4k.hdr";

    glusLogSetLevel(GLUS_LOG_DEBUG);

    glusWindowSetInitFunc(init);
    glusWindowSetReshapeFunc(reshape);
    glusWindowSetUpdateFunc(update);
    glusWindowSetTerminateFunc(terminate);
    glusWindowSetKeyFunc(key);

    if (!glusWindowCreate("GLUS Example Window",
                          SCREEN_WIDTH, SCREEN_HEIGHT,
                          GLUS_FALSE, GLUS_FALSE,
                          eglConfigAttributes, eglContextAttributes, 0))
    {
        printf("Failed to create window\n");
        return -1;
    }

    glusWindowRun();

    return 0;
}
