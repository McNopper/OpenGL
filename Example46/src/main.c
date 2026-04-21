/**
 * OpenGL 4 - Example 46
 *
 * @author	Norbert Nopper norbert@nopper.tv
 *
 * Homepage: http://nopper.tv
 *
 * Copyright Norbert Nopper
 *
 * Demonstrates real-time global illumination via Voxel Cone Tracing (VCT).
 *
 * The scene is first voxelised into a 64^3 RGBA8 radiance volume (mip-mapped).
 * Each frame the Sponza atrium is rendered with a cone-tracing fragment shader
 * that traces hemisphere cones for indirect diffuse, a specular cone and an
 * ambient-occlusion estimate, all sourced from the voxel texture.
 *
 * Reference:
 * Cyril Crassin, Fabrice Neyret, Miguel Sainz, Simon Green, Elmar Eisemann,
 * "Interactive Indirect Illumination Using Voxel Cone Tracing",
 * Pacific Graphics 2011.
 * https://research.nvidia.com/sites/default/files/pubs/2011-09_Interactive-Indirect-Illumination/GIVoxels-pg2011-authors.pdf
 *
 * Camera controls:
 *   W / S  - move forward / backward
 *   A / D  - rotate left / right
 *   V      - re-voxelise the scene
 */

#include <math.h>
#include <stdio.h>

#include "GL/glus.h"

/* -------------------------------------------------------------------------
 * Constants
 * ---------------------------------------------------------------------- */

#define WINDOW_WIDTH    1024
#define WINDOW_HEIGHT   768

/* Resolution of the 3-D voxel grid (must be a power of two). */
#define VCT_GRID_SIZE   64

/* Number of mip levels = log2(VCT_GRID_SIZE) + 1. */
#define VCT_MIPLEVELS   7

/* World space is normalised so the scene fits inside [-1, 1]^3. */
#define VCT_WORLD_SIZE  2.0f

/* Sponza bounding box: X [-192.09, 179.99], Y [-12.64, 142.94], Z [-118.28, 110.54]
 * Scale = 2 / max_range = 2 / 372.08 ~= 0.005375
 * Translate = -center = (6.05, -65.15, 3.87) applied before scale. */
#define SPONZA_SCALE    0.005375f
#define SPONZA_TX       6.05f
#define SPONZA_TY      -65.15f
#define SPONZA_TZ       3.87f

/* Attribute locations shared by both shader programs. */
#define LOCATION_VERTEX   0
#define LOCATION_NORMAL   1
#define LOCATION_TEXCOORD 2

/* Texture/image unit bindings. */
#define BINDING_VOXEL_GRID   0   /* image3D (voxelise) / sampler3D (vct) */
#define BINDING_DIFFUSE_TEX  1   /* sampler2D per-material diffuse        */

/* -------------------------------------------------------------------------
 * Shader programs
 * ---------------------------------------------------------------------- */

/* Pass 1: voxelisation (vert + geom + frag, writes to image3D). */
static GLUSprogram g_voxelizeProgram;

static GLint g_voxelize_modelMatrixLoc;
static GLint g_voxelize_lightPosLoc;
static GLint g_voxelize_lightColorLoc;
static GLint g_voxelize_diffuseColorLoc;
static GLint g_voxelize_hasDiffuseTextureLoc;
static GLint g_voxelize_voxelGridSizeLoc;

/* Pass 2: VCT rendering (vert + frag, samples sampler3D). */
static GLUSprogram g_vctProgram;

static GLint g_vct_modelMatrixLoc;
static GLint g_vct_mvpMatrixLoc;
static GLint g_vct_cameraPosLoc;
static GLint g_vct_lightPosLoc;
static GLint g_vct_lightColorLoc;
static GLint g_vct_diffuseColorLoc;
static GLint g_vct_specularColorLoc;
static GLint g_vct_shininessLoc;
static GLint g_vct_hasDiffuseTextureLoc;
static GLint g_vct_voxelGridWorldSizeLoc;
static GLint g_vct_voxelDimensionsLoc;

/* -------------------------------------------------------------------------
 * Scene data
 * ---------------------------------------------------------------------- */

static GLUSwavefront g_wavefront;

static GLuint g_voxelGrid;    /* RGBA8 3-D texture, VCT_MIPLEVELS mip levels */

/* Model matrix (uniform scale + translate). */
static GLfloat g_modelMatrix[16];

/* -------------------------------------------------------------------------
 * Camera state
 * ---------------------------------------------------------------------- */

/* Eye position in normalised world space.
 * Standing on the ground floor of the Sponza corridor, looking along +X
 * so that the hanging flags on the colonnade are visible. */
static GLfloat g_eye[3] = { 0.0f, -0.30f, 0.0f };

/* Horizontal yaw in degrees.  0 = looking along -Z; 90 = looking along +X. */
static GLfloat g_yaw = 90.0f;

/* Key states for continuous camera movement. */
static GLboolean g_moveForward  = GLUS_FALSE;
static GLboolean g_moveBackward = GLUS_FALSE;
static GLboolean g_turnLeft     = GLUS_FALSE;
static GLboolean g_turnRight    = GLUS_FALSE;

/* -------------------------------------------------------------------------
 * Voxelisation flag
 * ---------------------------------------------------------------------- */

/* Set to GLUS_TRUE to (re-)voxelise on the next frame. */
static GLboolean g_needsVoxelization = GLUS_TRUE;

/* -------------------------------------------------------------------------
 * Window dimensions (updated by reshape)
 * ---------------------------------------------------------------------- */
static GLint g_windowWidth  = WINDOW_WIDTH;
static GLint g_windowHeight = WINDOW_HEIGHT;

/* =========================================================================
 * Key handler
 * ====================================================================== */

GLUSvoid key(const GLUSboolean pressed, const GLUSint k)
{
    if (k == 'w') g_moveForward  = pressed;
    if (k == 's') g_moveBackward = pressed;
    if (k == 'a') g_turnLeft     = pressed;
    if (k == 'd') g_turnRight    = pressed;

    /* Press 'v' to trigger a manual re-voxelisation. */
    if (k == 'v' && pressed)
        g_needsVoxelization = GLUS_TRUE;
}

/* =========================================================================
 * Init
 * ====================================================================== */

GLUSboolean init(GLUSvoid)
{
    GLUStextfile vertexSource;
    GLUStextfile geometrySource;
    GLUStextfile fragmentSource;
    GLUStgaimage  image;

    GLUSgroupList*    groupWalker;
    GLUSmaterialList* materialWalker;


    /* ------------------------------------------------------------------
     * Build voxelisation program (vert + geom + frag).
     * ---------------------------------------------------------------- */

    glusFileLoadText("../Example46/shader/voxelize.vert.glsl", &vertexSource);
    glusFileLoadText("../Example46/shader/voxelize.geom.glsl", &geometrySource);
    glusFileLoadText("../Example46/shader/voxelize.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_voxelizeProgram,
                               (const GLUSchar**) &vertexSource.text,
                               0, 0,
                               (const GLUSchar**) &geometrySource.text,
                               (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&geometrySource);
    glusFileDestroyText(&fragmentSource);

    glUseProgram(g_voxelizeProgram.program);

    g_voxelize_modelMatrixLoc        = glGetUniformLocation(g_voxelizeProgram.program, "u_modelMatrix");
    g_voxelize_lightPosLoc           = glGetUniformLocation(g_voxelizeProgram.program, "u_lightPos");
    g_voxelize_lightColorLoc         = glGetUniformLocation(g_voxelizeProgram.program, "u_lightColor");
    g_voxelize_diffuseColorLoc       = glGetUniformLocation(g_voxelizeProgram.program, "u_diffuseColor");
    g_voxelize_hasDiffuseTextureLoc  = glGetUniformLocation(g_voxelizeProgram.program, "u_hasDiffuseTexture");
    g_voxelize_voxelGridSizeLoc      = glGetUniformLocation(g_voxelizeProgram.program, "u_voxelGridSize");

    /* Static uniforms set once. */
    glUniform3f(g_voxelize_lightPosLoc,   0.0f,  0.38f, 0.0f);
    glUniform3f(g_voxelize_lightColorLoc, 2.0f,  2.0f,  2.0f);
    glUniform1i(g_voxelize_voxelGridSizeLoc, VCT_GRID_SIZE);

    glUseProgram(0);


    /* ------------------------------------------------------------------
     * Build VCT rendering program (vert + frag).
     * ---------------------------------------------------------------- */

    glusFileLoadText("../Example46/shader/vct.vert.glsl", &vertexSource);
    glusFileLoadText("../Example46/shader/vct.frag.glsl", &fragmentSource);

    glusProgramBuildFromSource(&g_vctProgram,
                               (const GLUSchar**) &vertexSource.text,
                               0, 0, 0,
                               (const GLUSchar**) &fragmentSource.text);

    glusFileDestroyText(&vertexSource);
    glusFileDestroyText(&fragmentSource);

    glUseProgram(g_vctProgram.program);

    g_vct_modelMatrixLoc        = glGetUniformLocation(g_vctProgram.program, "u_modelMatrix");
    g_vct_mvpMatrixLoc          = glGetUniformLocation(g_vctProgram.program, "u_mvpMatrix");
    g_vct_cameraPosLoc          = glGetUniformLocation(g_vctProgram.program, "u_cameraPos");
    g_vct_lightPosLoc           = glGetUniformLocation(g_vctProgram.program, "u_lightPos");
    g_vct_lightColorLoc         = glGetUniformLocation(g_vctProgram.program, "u_lightColor");
    g_vct_diffuseColorLoc       = glGetUniformLocation(g_vctProgram.program, "u_diffuseColor");
    g_vct_specularColorLoc      = glGetUniformLocation(g_vctProgram.program, "u_specularColor");
    g_vct_shininessLoc          = glGetUniformLocation(g_vctProgram.program, "u_shininess");
    g_vct_hasDiffuseTextureLoc  = glGetUniformLocation(g_vctProgram.program, "u_hasDiffuseTexture");
    g_vct_voxelGridWorldSizeLoc = glGetUniformLocation(g_vctProgram.program, "u_voxelGridWorldSize");
    g_vct_voxelDimensionsLoc    = glGetUniformLocation(g_vctProgram.program, "u_voxelDimensions");

    /* Static uniforms set once. */
    glUniform3f(g_vct_lightPosLoc,          0.0f,  0.38f, 0.0f);
    glUniform3f(g_vct_lightColorLoc,        2.0f,  2.0f,  2.0f);
    glUniform1f(g_vct_voxelGridWorldSizeLoc, VCT_WORLD_SIZE);
    glUniform1i(g_vct_voxelDimensionsLoc,    VCT_GRID_SIZE);

    glUseProgram(0);


    /* ------------------------------------------------------------------
     * Load the Sponza wavefront model.
     * ---------------------------------------------------------------- */

    if (!glusWavefrontLoad("sponza.obj", &g_wavefront))
    {
        printf("Could not load sponza.obj\n");
        return GLUS_FALSE;
    }


    /* Vertices VBO (4 floats per vertex). */
    glGenBuffers(1, &g_wavefront.verticesVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.verticesVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 g_wavefront.numberVertices * 4 * sizeof(GLfloat),
                 (GLfloat*) g_wavefront.vertices,
                 GL_STATIC_DRAW);

    /* Normals VBO (3 floats per vertex). */
    glGenBuffers(1, &g_wavefront.normalsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.normalsVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 g_wavefront.numberVertices * 3 * sizeof(GLfloat),
                 (GLfloat*) g_wavefront.normals,
                 GL_STATIC_DRAW);

    /* Texture coordinates VBO (2 floats per vertex). */
    glGenBuffers(1, &g_wavefront.texCoordsVBO);
    glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.texCoordsVBO);
    glBufferData(GL_ARRAY_BUFFER,
                 g_wavefront.numberVertices * 2 * sizeof(GLfloat),
                 (GLfloat*) g_wavefront.texCoords,
                 GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);


    /* ------------------------------------------------------------------
     * Create per-group index buffers and VAOs.
     * Both programs use the same attribute locations so a single VAO
     * per group suffices.
     * ---------------------------------------------------------------- */

    glUseProgram(g_vctProgram.program);

    groupWalker = g_wavefront.groups;
    while (groupWalker)
    {
        glGenBuffers(1, &groupWalker->group.indicesVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     groupWalker->group.numberIndices * sizeof(GLuint),
                     (GLuint*) groupWalker->group.indices,
                     GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        glGenVertexArrays(1, &groupWalker->group.vao);
        glBindVertexArray(groupWalker->group.vao);

        glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.verticesVBO);
        glVertexAttribPointer(LOCATION_VERTEX, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(LOCATION_VERTEX);

        glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.normalsVBO);
        glVertexAttribPointer(LOCATION_NORMAL, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(LOCATION_NORMAL);

        glBindBuffer(GL_ARRAY_BUFFER, g_wavefront.texCoordsVBO);
        glVertexAttribPointer(LOCATION_TEXCOORD, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(LOCATION_TEXCOORD);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, groupWalker->group.indicesVBO);

        glBindVertexArray(0);

        groupWalker = groupWalker->next;
    }

    glUseProgram(0);


    /* ------------------------------------------------------------------
     * Load per-material diffuse textures.
     * ---------------------------------------------------------------- */

    glActiveTexture(GL_TEXTURE0 + BINDING_DIFFUSE_TEX);

    materialWalker = g_wavefront.materials;
    while (materialWalker)
    {
        if (materialWalker->material.diffuseTextureFilename[0] != '\0')
        {
            if (!glusImageLoadTga(materialWalker->material.diffuseTextureFilename, &image))
            {
                /* Non-fatal: some materials have no texture. */
                printf("Warning: could not load texture %s\n",
                       materialWalker->material.diffuseTextureFilename);
                materialWalker = materialWalker->next;
                continue;
            }

            glGenTextures(1, &materialWalker->material.diffuseTextureName);
            glBindTexture(GL_TEXTURE_2D, materialWalker->material.diffuseTextureName);

            glTexImage2D(GL_TEXTURE_2D, 0,
                         image.format, image.width, image.height, 0,
                         image.format, GL_UNSIGNED_BYTE, image.data);

            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

            glBindTexture(GL_TEXTURE_2D, 0);

            glusImageDestroyTga(&image);
        }

        materialWalker = materialWalker->next;
    }

    glActiveTexture(GL_TEXTURE0);


    /* ------------------------------------------------------------------
     * Create the RGBA8 3-D voxel radiance texture.
     * ---------------------------------------------------------------- */

    glGenTextures(1, &g_voxelGrid);
    glBindTexture(GL_TEXTURE_3D, g_voxelGrid);

    glTexStorage3D(GL_TEXTURE_3D, VCT_MIPLEVELS, GL_RGBA8,
                   VCT_GRID_SIZE, VCT_GRID_SIZE, VCT_GRID_SIZE);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_3D, 0);

    /* ------------------------------------------------------------------
     * Model matrix: first translate to centre the scene, then scale to
     * fit into [-1, 1]^3.  In GLUS the operations post-multiply, so we
     * apply Scale first and Translate second: M = Scale * Translate.
     * For a vertex v:  M*v = Scale * (v + translate) = s*(v - center).
     * ---------------------------------------------------------------- */

    glusMatrix4x4Identityf(g_modelMatrix);
    glusMatrix4x4Scalef(g_modelMatrix, SPONZA_SCALE, SPONZA_SCALE, SPONZA_SCALE);
    glusMatrix4x4Translatef(g_modelMatrix, SPONZA_TX, SPONZA_TY, SPONZA_TZ);

    /* ------------------------------------------------------------------
     * Global OpenGL state.
     * ---------------------------------------------------------------- */

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);


    return GLUS_TRUE;
}

/* =========================================================================
 * Reshape
 * ====================================================================== */

GLUSvoid reshape(GLUSint width, GLUSint height)
{
    g_windowWidth  = width;
    g_windowHeight = height;
}

/* =========================================================================
 * Update (called every frame)
 * ====================================================================== */

GLUSboolean update(GLUSfloat time)
{
    GLUSgroupList* groupWalker;

    GLfloat viewMatrix[16];
    GLfloat projectionMatrix[16];
    GLfloat mvpMatrix[16];
    GLfloat vpMatrix[16];

    static const GLubyte clearValue[4] = { 0, 0, 0, 0 };
    /* ------------------------------------------------------------------
     * Camera update
     * ---------------------------------------------------------------- */

    {
        GLfloat moveSpeed = 0.5f * time;  /* world units per second */
        GLfloat turnSpeed = 60.0f * time; /* degrees per second     */

        if (g_turnLeft)  g_yaw -= turnSpeed;
        if (g_turnRight) g_yaw += turnSpeed;

        /* Forward direction in world XZ plane. */
        GLfloat yawRad = g_yaw * GLUS_PI / 180.0f;
        GLfloat fwdX   =  sinf(yawRad);
        GLfloat fwdZ   = -cosf(yawRad);

        if (g_moveForward)  { g_eye[0] += fwdX * moveSpeed; g_eye[2] += fwdZ * moveSpeed; }
        if (g_moveBackward) { g_eye[0] -= fwdX * moveSpeed; g_eye[2] -= fwdZ * moveSpeed; }

        /* Look-at target is one unit ahead along the forward vector. */
        glusMatrix4x4LookAtf(viewMatrix,
                             g_eye[0],          g_eye[1],          g_eye[2],
                             g_eye[0] + fwdX,   g_eye[1],          g_eye[2] + fwdZ,
                             0.0f, 1.0f, 0.0f);

        glusMatrix4x4Perspectivef(projectionMatrix,
                                  60.0f,
                                  (GLfloat) g_windowWidth / (GLfloat) g_windowHeight,
                                  0.001f, 10.0f);

        glusMatrix4x4Multiplyf(vpMatrix, projectionMatrix, viewMatrix);
        glusMatrix4x4Multiplyf(mvpMatrix, vpMatrix, g_modelMatrix);
    }

    /* ------------------------------------------------------------------
     * Voxelisation pass (runs only when g_needsVoxelization is set).
     * ---------------------------------------------------------------- */

    if (g_needsVoxelization)
    {
        g_needsVoxelization = GLUS_FALSE;

        /* Clear the base mip level of the voxel grid before re-voxelising. */
        glBindTexture(GL_TEXTURE_3D, g_voxelGrid);
        glClearTexImage(g_voxelGrid, 0, GL_RGBA, GL_UNSIGNED_BYTE, clearValue);
        glBindTexture(GL_TEXTURE_3D, 0);

        /* State for voxelisation: no colour output, no depth test, no culling. */
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glViewport(0, 0, VCT_GRID_SIZE, VCT_GRID_SIZE);

        glUseProgram(g_voxelizeProgram.program);

        glUniformMatrix4fv(g_voxelize_modelMatrixLoc, 1, GL_FALSE, g_modelMatrix);

        /* Bind the voxel grid as a write-only image at binding point 0. */
        glBindImageTexture(BINDING_VOXEL_GRID, g_voxelGrid, 0, GL_TRUE, 0,
                           GL_WRITE_ONLY, GL_RGBA8);

        glActiveTexture(GL_TEXTURE0 + BINDING_DIFFUSE_TEX);

        groupWalker = g_wavefront.groups;
        while (groupWalker)
        {
            if (groupWalker->group.material)
            {
                glUniform4fv(g_voxelize_diffuseColorLoc, 1,
                             groupWalker->group.material->diffuse);

                if (groupWalker->group.material->diffuseTextureName)
                {
                    glBindTexture(GL_TEXTURE_2D,
                                  groupWalker->group.material->diffuseTextureName);
                    glUniform1i(g_voxelize_hasDiffuseTextureLoc, 1);
                }
                else
                {
                    glBindTexture(GL_TEXTURE_2D, 0);
                    glUniform1i(g_voxelize_hasDiffuseTextureLoc, 0);
                }
            }
            else
            {
                glUniform4f(g_voxelize_diffuseColorLoc, 0.8f, 0.8f, 0.8f, 1.0f);
                glBindTexture(GL_TEXTURE_2D, 0);
                glUniform1i(g_voxelize_hasDiffuseTextureLoc, 0);
            }

            glBindVertexArray(groupWalker->group.vao);
            glDrawElements(GL_TRIANGLES,
                           groupWalker->group.numberIndices,
                           GL_UNSIGNED_INT, 0);

            groupWalker = groupWalker->next;
        }

        glBindVertexArray(0);
        glActiveTexture(GL_TEXTURE0);

        /* Unbind image. */
        glBindImageTexture(BINDING_VOXEL_GRID, 0, 0, GL_TRUE, 0,
                           GL_WRITE_ONLY, GL_RGBA8);

        /* Ensure all image writes are visible before mip generation. */
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT |
                        GL_TEXTURE_FETCH_BARRIER_BIT);

        /* Generate mip chain for cone tracing (lodLevel sampling). */
        glBindTexture(GL_TEXTURE_3D, g_voxelGrid);
        glGenerateMipmap(GL_TEXTURE_3D);
        glBindTexture(GL_TEXTURE_3D, 0);

        /* Restore rendering state. */
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glViewport(0, 0, g_windowWidth, g_windowHeight);
    }

    /* ------------------------------------------------------------------
     * VCT rendering pass
     * ---------------------------------------------------------------- */

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(g_vctProgram.program);

    glUniformMatrix4fv(g_vct_modelMatrixLoc, 1, GL_FALSE, g_modelMatrix);
    glUniformMatrix4fv(g_vct_mvpMatrixLoc,   1, GL_FALSE, mvpMatrix);
    glUniform3fv(g_vct_cameraPosLoc, 1, g_eye);

    /* Bind the voxel grid as a trilinear-mipmapped sampler at unit 0. */
    glActiveTexture(GL_TEXTURE0 + BINDING_VOXEL_GRID);
    glBindTexture(GL_TEXTURE_3D, g_voxelGrid);

    glActiveTexture(GL_TEXTURE0 + BINDING_DIFFUSE_TEX);

    groupWalker = g_wavefront.groups;
    while (groupWalker)
    {
        if (groupWalker->group.material)
        {
            glUniform4fv(g_vct_diffuseColorLoc,  1, groupWalker->group.material->diffuse);
            glUniform4fv(g_vct_specularColorLoc, 1, groupWalker->group.material->specular);
            glUniform1f(g_vct_shininessLoc,         groupWalker->group.material->shininess);

            if (groupWalker->group.material->diffuseTextureName)
            {
                glBindTexture(GL_TEXTURE_2D,
                              groupWalker->group.material->diffuseTextureName);
                glUniform1i(g_vct_hasDiffuseTextureLoc, 1);
            }
            else
            {
                glBindTexture(GL_TEXTURE_2D, 0);
                glUniform1i(g_vct_hasDiffuseTextureLoc, 0);
            }
        }
        else
        {
            glUniform4f(g_vct_diffuseColorLoc,  0.8f, 0.8f, 0.8f, 1.0f);
            glUniform4f(g_vct_specularColorLoc, 0.0f, 0.0f, 0.0f, 1.0f);
            glUniform1f(g_vct_shininessLoc, 10.0f);
            glBindTexture(GL_TEXTURE_2D, 0);
            glUniform1i(g_vct_hasDiffuseTextureLoc, 0);
        }

        glBindVertexArray(groupWalker->group.vao);
        glDrawElements(GL_TRIANGLES,
                       groupWalker->group.numberIndices,
                       GL_UNSIGNED_INT, 0);

        groupWalker = groupWalker->next;
    }

    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0 + BINDING_VOXEL_GRID);
    glBindTexture(GL_TEXTURE_3D, 0);
    glActiveTexture(GL_TEXTURE0);

    return GLUS_TRUE;
}

/* =========================================================================
 * Terminate
 * ====================================================================== */

GLUSvoid terminate(GLUSvoid)
{
    GLUSgroupList*    groupWalker;
    GLUSmaterialList* materialWalker;

    /* Voxel grid texture. */
    glBindTexture(GL_TEXTURE_3D, 0);
    if (g_voxelGrid)
    {
        glDeleteTextures(1, &g_voxelGrid);
        g_voxelGrid = 0;
    }

    /* VBOs. */
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    if (g_wavefront.verticesVBO)
    {
        glDeleteBuffers(1, &g_wavefront.verticesVBO);
        g_wavefront.verticesVBO = 0;
    }
    if (g_wavefront.normalsVBO)
    {
        glDeleteBuffers(1, &g_wavefront.normalsVBO);
        g_wavefront.normalsVBO = 0;
    }
    if (g_wavefront.texCoordsVBO)
    {
        glDeleteBuffers(1, &g_wavefront.texCoordsVBO);
        g_wavefront.texCoordsVBO = 0;
    }

    /* Per-group VAOs and index buffers. */
    glBindVertexArray(0);

    groupWalker = g_wavefront.groups;
    while (groupWalker)
    {
        if (groupWalker->group.indicesVBO)
        {
            glDeleteBuffers(1, &groupWalker->group.indicesVBO);
            groupWalker->group.indicesVBO = 0;
        }
        if (groupWalker->group.vao)
        {
            glDeleteVertexArrays(1, &groupWalker->group.vao);
            groupWalker->group.vao = 0;
        }
        groupWalker = groupWalker->next;
    }

    /* Per-material textures. */
    glBindTexture(GL_TEXTURE_2D, 0);

    materialWalker = g_wavefront.materials;
    while (materialWalker)
    {
        if (materialWalker->material.diffuseTextureName)
        {
            glDeleteTextures(1, &materialWalker->material.diffuseTextureName);
            materialWalker->material.diffuseTextureName = 0;
        }
        materialWalker = materialWalker->next;
    }

    /* Shader programs. */
    glUseProgram(0);
    glusProgramDestroy(&g_voxelizeProgram);
    glusProgramDestroy(&g_vctProgram);

    glusWavefrontDestroy(&g_wavefront);
}

/* =========================================================================
 * Entry point
 * ====================================================================== */

int main(int argc, char* argv[])
{
    EGLint eglConfigAttributes[] = {
        EGL_RED_SIZE,   8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE,  8,
        EGL_DEPTH_SIZE, 24,
        EGL_STENCIL_SIZE, 0,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
        EGL_NONE
    };

    EGLint eglContextAttributes[] = {
        EGL_CONTEXT_MAJOR_VERSION, 4,
        EGL_CONTEXT_MINOR_VERSION, 4,
        EGL_CONTEXT_OPENGL_FORWARD_COMPATIBLE, EGL_TRUE,
        EGL_CONTEXT_OPENGL_PROFILE_MASK, EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT,
        EGL_CONTEXT_OPENGL_DEBUG, EGL_TRUE,
        EGL_NONE
    };

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

    glusWindowRun();

    return 0;
}
