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

#ifndef GLUS_IBL_H_
#define GLUS_IBL_H_

/**
 * Set the directory in which the GLUS IBL shaders reside.
 * The default path is "../GLUS/shader/".
 *
 * @param path  Null-terminated path string including trailing slash/backslash.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusIblSetShaderPath(const GLUSchar* path);

/**
 * Convert a latitude-longitude HDR panorama texture to a plain cubemap.
 * The result is suitable for use as a scene background.
 *
 * @param backgroundTexture  [out] GL texture name of the created GL_TEXTURE_CUBE_MAP (GL_RGB32F).
 * @param panoramaTexture    Bound GL_TEXTURE_2D containing the equirectangular panorama.
 * @param size               Edge length (pixels) of each cubemap face.
 * @return GLUS_TRUE on success, GLUS_FALSE on failure.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusIblBuildBackgroundCubemap(GLUSuint* backgroundTexture, GLUSuint panoramaTexture, GLUSint size);

/**
 * Pre-filter the environment map for specular IBL into a cubemap-array texture
 * where layer i*6+face contains the face image at roughness level i.
 *
 * @param specularTexture   [out] GL texture name of the created GL_TEXTURE_CUBE_MAP_ARRAY (GL_RGB32F).
 * @param panoramaTexture   Bound GL_TEXTURE_2D containing the equirectangular panorama.
 * @param size              Edge length (pixels) of the mip-0 face.
 * @param numberRoughness   Number of roughness levels (layers = numberRoughness * 6).
 * @return GLUS_TRUE on success, GLUS_FALSE on failure.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusIblBuildSpecularEnvironmentMap(GLUSuint* specularTexture, GLUSuint panoramaTexture, GLUSint size, GLUSint numberRoughness);

/**
 * Convolve the environment map with a cosine lobe to produce a diffuse
 * irradiance cubemap.
 *
 * @param diffuseTexture   [out] GL texture name of the created GL_TEXTURE_CUBE_MAP (GL_RGB32F).
 * @param panoramaTexture  Bound GL_TEXTURE_2D containing the equirectangular panorama.
 * @param size             Edge length (pixels) of each cubemap face.
 * @return GLUS_TRUE on success, GLUS_FALSE on failure.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusIblBuildDiffuseEnvironmentMap(GLUSuint* diffuseTexture, GLUSuint panoramaTexture, GLUSint size);

/**
 * Pre-integrate the split-sum BRDF look-up table.
 *
 * @param brdfLutTexture   [out] GL texture name of the created GL_TEXTURE_2D (GL_RG32F).
 * @param size             Width/height of the square LUT texture.
 * @return GLUS_TRUE on success, GLUS_FALSE on failure.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusIblBuildBrdfLookupTable(GLUSuint* brdfLutTexture, GLUSint size);

#endif /* GLUS_IBL_H_ */
