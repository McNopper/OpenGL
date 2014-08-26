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

#ifndef GLUS_PERLIN_H_
#define GLUS_PERLIN_H_

/**
 * Creates a 1D perlin noise texture. See OpenGL Programming Guide 4.3, p.460ff
 *
 * @param image The perlin noise texture will be stored into this image.
 * @param width Width of the texture.
 * @param seed Random seed number.
 * @param frequency Frequency of the noise.
 * @param amplitude Amplitude of the noise.
 * @param persistence Persistence of the noise.
 * @param octaves Octaves of the noise.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusPerlinCreateNoise1D(GLUStgaimage* image, const GLUSint width, const GLUSint seed, const GLUSfloat frequency, const GLUSfloat amplitude, const GLUSfloat persistence, const GLUSint octaves);

/**
 * Creates a 2D perlin noise texture. See OpenGL Programming Guide 4.3, p.460ff
 *
 * @param image The perlin noise texture will be stored into this image.
 * @param width Width of the texture.
 * @param height Height of the texture
 * @param seed Random seed number.
 * @param frequency Frequency of the noise.
 * @param amplitude Amplitude of the noise.
 * @param persistence Persistence of the noise.
 * @param octaves Octaves of the noise.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusPerlinCreateNoise2D(GLUStgaimage* image, const GLUSint width, const GLUSint height, const GLUSint seed, const GLUSfloat frequency, const GLUSfloat amplitude, const GLUSfloat persistence, const GLUSint octaves);

/**
 * Creates a 3D perlin noise texture. See OpenGL Programming Guide 4.3, p.460ff
 *
 * @param image The perlin noise texture will be stored into this image.
 * @param width Width of the texture.
 * @param height Height of the texture
 * @param depth Depth of the texture
 * @param seed Random seed number.
 * @param frequency Frequency of the noise.
 * @param amplitude Amplitude of the noise.
 * @param persistence Persistence of the noise.
 * @param octaves Octaves of the noise.
 *
 * @return GLUS_TRUE, if creation succeeded.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusPerlinCreateNoise3D(GLUStgaimage* image, const GLUSint width, const GLUSint height, const GLUSint depth, const GLUSint seed, const GLUSfloat frequency, const GLUSfloat amplitude, const GLUSfloat persistence, const GLUSint octaves);

#endif /* GLUS_PERLIN_H_ */
