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

#ifndef GLUS_MATRIX_PLANAR_H_
#define GLUS_MATRIX_PLANAR_H_

/**
 * Creates the projection matrix to simulate a planar shadow coming from a point / spot light.
 *
 * @param matrix The matrix, which is set to the projection matrix.
 * @param shadowPlane The shadow receiver plane.
 * @param lightPoint The origin of the light.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4PlanarShadowPointLightf(GLUSfloat matrix[16], const GLUSfloat shadowPlane[4], const GLUSfloat lightPoint[4]);

/**
 * Creates the projection matrix to simulate a planar shadow coming from a directional light.
 *
 * @param matrix The matrix, which is set to the projection matrix.
 * @param shadowPlane The shadow receiver plane.
 * @param lightDirection The direction of the light, pointing towards the light source.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4PlanarShadowDirectionalLightf(GLUSfloat matrix[16], const GLUSfloat shadowPlane[4], const GLUSfloat lightDirection[3]);

/**
 * Creates the projection matrix to simulate a planar reflection.
 *
 * @param matrix The matrix, which is set to the projection matrix.
 * @param reflectionPlane The reflecting plane.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusMatrix4x4PlanarReflectionf(GLUSfloat matrix[16], const GLUSfloat reflectionPlane[4]);

#endif /* GLUS_MATRIX_PLANAR_H_ */
