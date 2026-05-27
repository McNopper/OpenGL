/*
 * GLUS - Modern OpenGL, OpenGL ES and OpenVG Utilities. Copyright (C) since 2010 Norbert Nopper
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "GL/glus.h"

#include <string.h>
#include <stdio.h>

// Maximum length for shader path + filename.
#define GLUS_IBL_MAX_PATH 512

// Intermediate source cubemap size for the panorama-to-cubemap conversion step.
// 512 provides a good quality/cost trade-off; the final filtered maps are
// typically 256 (specular) and 64–128 (diffuse).
#define IBL_SOURCE_CUBEMAP_SIZE 512

// Default relative path to GLUS IBL shaders.
static GLUSchar g_iblShaderPath[GLUS_IBL_MAX_PATH] = "../GLUS/shader/";

// ---------------------------------------------------------------------------
// Internal helper — build an OpenGL program from two on-disk GLSL files.
// Returns GLUS_FALSE and leaves program unchanged on any error.
// ---------------------------------------------------------------------------
static GLUSboolean iblBuildProgram(GLUSprogram* program, const GLUSchar* vertFile, const GLUSchar* fragFile)
{
    GLUSchar     vertPath[GLUS_IBL_MAX_PATH];
    GLUSchar     fragPath[GLUS_IBL_MAX_PATH];
    GLUStextfile vertSource;
    GLUStextfile fragSource;
    GLUSboolean  result;

    memset(vertPath, 0, sizeof(vertPath));
    memset(fragPath, 0, sizeof(fragPath));
    memset(&vertSource, 0, sizeof(vertSource));
    memset(&fragSource, 0, sizeof(fragSource));

    strncpy(vertPath, g_iblShaderPath, GLUS_IBL_MAX_PATH - 1);
    strncat(vertPath, vertFile, GLUS_IBL_MAX_PATH - strlen(vertPath) - 1);

    strncpy(fragPath, g_iblShaderPath, GLUS_IBL_MAX_PATH - 1);
    strncat(fragPath, fragFile, GLUS_IBL_MAX_PATH - strlen(fragPath) - 1);

    if (!glusFileLoadText(vertPath, &vertSource))
    {
        printf("glus_ibl: could not load %s\n", vertPath);
        return GLUS_FALSE;
    }

    if (!glusFileLoadText(fragPath, &fragSource))
    {
        printf("glus_ibl: could not load %s\n", fragPath);
        glusFileDestroyText(&vertSource);
        return GLUS_FALSE;
    }

    result = glusProgramBuildFromSource(program,
                                        (const GLUSchar**)&vertSource.text,
                                        0, 0, 0,
                                        (const GLUSchar**)&fragSource.text);

    glusFileDestroyText(&vertSource);
    glusFileDestroyText(&fragSource);

    return result;
}

// ---------------------------------------------------------------------------
// Internal helper — count mip levels for a square texture of the given size.
// ---------------------------------------------------------------------------
static GLUSint iblMipLevels(GLUSint size)
{
    GLUSint levels = 1;
    while (size > 1)
    {
        size >>= 1;
        ++levels;
    }
    return levels;
}

// ---------------------------------------------------------------------------
// Internal helper — convert the equirectangular panorama to a mipmapped
// GL_TEXTURE_CUBE_MAP.  The caller must delete the returned texture when done.
// ---------------------------------------------------------------------------
static GLUSboolean buildSourceCubemap(GLUSuint* cubemapOut, GLUSuint panoramaTexture, GLUSint size)
{
    GLUSprogram program;
    GLUSuint    fbo, vao, tex;
    GLUSint     locPanorama, locFace;
    GLUSint     face;
    GLUSint     savedViewport[4];

    if (!iblBuildProgram(&program, "glus_ibl.vert.glsl", "glus_ibl_background.frag.glsl"))
    {
        return GLUS_FALSE;
    }

    locPanorama = glGetUniformLocation(program.program, "u_panoramaTexture");
    locFace     = glGetUniformLocation(program.program, "u_face");

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, iblMipLevels(size), GL_RGB32F, size, size);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glGenFramebuffers(1, &fbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glUseProgram(program.program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, panoramaTexture);
    glUniform1i(locPanorama, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGetIntegerv(GL_VIEWPORT, savedViewport);
    glViewport(0, 0, size, size);

    for (face = 0; face < 6; face++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, tex, 0);
        glUniform1i(locFace, face);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Generate the full mip chain so specular/diffuse shaders can use textureLod.
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteVertexArrays(1, &vao);
    glusProgramDestroy(&program);

    glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);

    *cubemapOut = tex;
    return GLUS_TRUE;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

GLUSvoid GLUSAPIENTRY glusIblSetShaderPath(const GLUSchar* path)
{
    if (!path)
    {
        return;
    }

    memset(g_iblShaderPath, 0, sizeof(g_iblShaderPath));
    strncpy(g_iblShaderPath, path, GLUS_IBL_MAX_PATH - 1);
}

// ---------------------------------------------------------------------------
// glusIblBuildBackgroundCubemap
// ---------------------------------------------------------------------------

GLUSboolean GLUSAPIENTRY glusIblBuildBackgroundCubemap(GLUSuint* backgroundTexture, GLUSuint panoramaTexture, GLUSint size)
{
    GLUSprogram program;
    GLUSuint    fbo;
    GLUSuint    vao;
    GLUSuint    tex;
    GLUSint     locPanorama;
    GLUSint     locFace;
    GLUSint     face;
    GLUSint     savedViewport[4];

    if (!backgroundTexture || !panoramaTexture || size <= 0)
    {
        return GLUS_FALSE;
    }

    // Build shader program.
    if (!iblBuildProgram(&program, "glus_ibl.vert.glsl", "glus_ibl_background.frag.glsl"))
    {
        return GLUS_FALSE;
    }

    locPanorama = glGetUniformLocation(program.program, "u_panoramaTexture");
    locFace     = glGetUniformLocation(program.program, "u_face");

    // Create the output cubemap texture.
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB32F, size, size);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // FBO + empty VAO for full-screen triangle-strip draw.
    glGenFramebuffers(1, &fbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glUseProgram(program.program);

    // Bind panorama to unit 0.
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, panoramaTexture);
    glUniform1i(locPanorama, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGetIntegerv(GL_VIEWPORT, savedViewport);
    glViewport(0, 0, size, size);

    for (face = 0; face < 6; face++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, tex, 0);
        glUniform1i(locFace, face);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    // Cleanup GPU state.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteVertexArrays(1, &vao);
    glusProgramDestroy(&program);

    glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);

    *backgroundTexture = tex;
    return GLUS_TRUE;
}

// ---------------------------------------------------------------------------
// glusIblBuildSpecularEnvironmentMap
// ---------------------------------------------------------------------------

GLUSboolean GLUSAPIENTRY glusIblBuildSpecularEnvironmentMap(GLUSuint* specularTexture, GLUSuint panoramaTexture, GLUSint size, GLUSint numberRoughness)
{
    GLUSprogram program;
    GLUSuint    fbo;
    GLUSuint    vao;
    GLUSuint    tex;
    GLUSuint    srcCubemap;
    GLUSint     locCubemap;
    GLUSint     locFace;
    GLUSint     locRoughness;
    GLUSint     locWidth;
    GLUSint     level;
    GLUSint     face;
    GLUSfloat   roughness;
    GLUSint     savedViewport[4];

    if (!specularTexture || !panoramaTexture || size <= 0 || numberRoughness <= 0)
    {
        return GLUS_FALSE;
    }

    if (!buildSourceCubemap(&srcCubemap, panoramaTexture, IBL_SOURCE_CUBEMAP_SIZE))
    {
        return GLUS_FALSE;
    }

    // Build shader program.
    if (!iblBuildProgram(&program, "glus_ibl.vert.glsl", "glus_ibl_specular.frag.glsl"))
    {
        glDeleteTextures(1, &srcCubemap);
        return GLUS_FALSE;
    }

    locCubemap   = glGetUniformLocation(program.program, "u_cubemapTexture");
    locFace      = glGetUniformLocation(program.program, "u_face");
    locRoughness = glGetUniformLocation(program.program, "u_roughness");
    locWidth     = glGetUniformLocation(program.program, "u_width");

    // GL_TEXTURE_CUBE_MAP_ARRAY: depth = numberRoughness * 6 layers.
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, tex);
    glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, 1, GL_RGB32F, size, size, numberRoughness * 6);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, 0);

    glGenFramebuffers(1, &fbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glUseProgram(program.program);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, srcCubemap);
    glUniform1i(locCubemap, 0);
    glUniform1i(locWidth, IBL_SOURCE_CUBEMAP_SIZE);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGetIntegerv(GL_VIEWPORT, savedViewport);
    glViewport(0, 0, size, size);

    for (level = 0; level < numberRoughness; level++)
    {
        roughness = (numberRoughness > 1) ? (GLUSfloat)level / (GLUSfloat)(numberRoughness - 1) : 0.0f;
        glUniform1f(locRoughness, roughness);

        for (face = 0; face < 6; face++)
        {
            // Layer index: each roughness level has 6 consecutive face layers.
            glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                      tex, 0, level * 6 + face);
            glUniform1i(locFace, face);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteVertexArrays(1, &vao);
    glusProgramDestroy(&program);
    glDeleteTextures(1, &srcCubemap);

    glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);

    *specularTexture = tex;
    return GLUS_TRUE;
}

// ---------------------------------------------------------------------------
// glusIblBuildDiffuseEnvironmentMap
// ---------------------------------------------------------------------------

GLUSboolean GLUSAPIENTRY glusIblBuildDiffuseEnvironmentMap(GLUSuint* diffuseTexture, GLUSuint panoramaTexture, GLUSint size)
{
    GLUSprogram program;
    GLUSuint    fbo;
    GLUSuint    vao;
    GLUSuint    tex;
    GLUSuint    srcCubemap;
    GLUSint     locCubemap;
    GLUSint     locFace;
    GLUSint     locWidth;
    GLUSint     face;
    GLUSint     savedViewport[4];

    if (!diffuseTexture || !panoramaTexture || size <= 0)
    {
        return GLUS_FALSE;
    }

    if (!buildSourceCubemap(&srcCubemap, panoramaTexture, IBL_SOURCE_CUBEMAP_SIZE))
    {
        return GLUS_FALSE;
    }

    if (!iblBuildProgram(&program, "glus_ibl.vert.glsl", "glus_ibl_diffuse.frag.glsl"))
    {
        glDeleteTextures(1, &srcCubemap);
        return GLUS_FALSE;
    }

    locCubemap = glGetUniformLocation(program.program, "u_cubemapTexture");
    locFace    = glGetUniformLocation(program.program, "u_face");
    locWidth   = glGetUniformLocation(program.program, "u_width");

    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_CUBE_MAP, tex);
    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGB32F, size, size);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    glGenFramebuffers(1, &fbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glUseProgram(program.program);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, srcCubemap);
    glUniform1i(locCubemap, 0);
    glUniform1i(locWidth, IBL_SOURCE_CUBEMAP_SIZE);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glGetIntegerv(GL_VIEWPORT, savedViewport);
    glViewport(0, 0, size, size);

    for (face = 0; face < 6; face++)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, tex, 0);
        glUniform1i(locFace, face);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteVertexArrays(1, &vao);
    glusProgramDestroy(&program);
    glDeleteTextures(1, &srcCubemap);

    glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);

    *diffuseTexture = tex;
    return GLUS_TRUE;
}

// ---------------------------------------------------------------------------
// glusIblBuildBrdfLookupTable
// ---------------------------------------------------------------------------

GLUSboolean GLUSAPIENTRY glusIblBuildBrdfLookupTable(GLUSuint* brdfLutTexture, GLUSint size)
{
    GLUSprogram program;
    GLUSuint    fbo;
    GLUSuint    vao;
    GLUSuint    tex;
    GLUSint     savedViewport[4];

    if (!brdfLutTexture || size <= 0)
    {
        return GLUS_FALSE;
    }

    if (!iblBuildProgram(&program, "glus_ibl.vert.glsl", "glus_ibl_brdf_lut.frag.glsl"))
    {
        return GLUS_FALSE;
    }

    // RG32F: red=scale (F0), green=bias.
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RG32F, size, size);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenFramebuffers(1, &fbo);
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glUseProgram(program.program);

    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);
    glGetIntegerv(GL_VIEWPORT, savedViewport);
    glViewport(0, 0, size, size);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(0);
    glUseProgram(0);

    glDeleteFramebuffers(1, &fbo);
    glDeleteVertexArrays(1, &vao);
    glusProgramDestroy(&program);

    glViewport(savedViewport[0], savedViewport[1], savedViewport[2], savedViewport[3]);

    *brdfLutTexture = tex;
    return GLUS_TRUE;
}
