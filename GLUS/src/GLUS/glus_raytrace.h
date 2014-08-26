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

#ifndef GLUS_RAYTRACE_H_
#define GLUS_RAYTRACE_H_

/**
 * Creates normals in a buffer for ray traced perspective projection. Directions are pointing to -Z direction.
 *
 * @param directionBuffer	The resulting direction buffer.
 * @param padding			Amount of padding bytes.
 * @param fovy				Field of view.
 * @param width				Width of the buffer.
 * @param height			Height of the buffer.
 *
 * @return GLUS_TRUE, if creation was successful.
 */
GLUSAPI GLUSboolean GLUSAPIENTRY glusRaytracePerspectivef(GLUSfloat* directionBuffer, const GLUSubyte padding, const GLUSfloat fovy, const GLUSint width, const GLUSint height);

/**
 * Creates the positions and directions in buffers needed for ray tracing.
 *
 * @param positionBuffer		The resulting position buffer. Positions are in homogeneous coordinates.
 * @param directionBuffer		The resulting direction buffer.
 * @param originDirectionBuffer	The direction buffer, pointing to -Z direction.
 * @param padding				Amount of padding bytes.
 * @param width 				The width of the buffers.
 * @param height 				The height of the buffers.
 * @param eyeX 					Eye / camera X position.
 * @param eyeY 					Eye / camera Y position.
 * @param eyeZ 					Eye / camera Z position.
 * @param centerX 				X Position, where the view / camera points to.
 * @param centerY 				Y Position, where the view / camera points to.
 * @param centerZ 				Z Position, where the view / camera points to.
 * @param upX 					Eye / camera X component from up vector.
 * @param upY 					Eye / camera Y component from up vector.
 * @param upZ 					Eye / camera Z component from up vector.
 */
GLUSAPI GLUSvoid GLUSAPIENTRY glusRaytraceLookAtf(GLUSfloat* positionBuffer, GLUSfloat* directionBuffer, const GLUSfloat* originDirectionBuffer, const GLUSubyte padding, const GLUSint width, const GLUSint height, const GLUSfloat eyeX, const GLUSfloat eyeY, const GLUSfloat eyeZ, const GLUSfloat centerX, const GLUSfloat centerY, const GLUSfloat centerZ, const GLUSfloat upX, const GLUSfloat upY, const GLUSfloat upZ);

#endif /* GLUS_RAYTRACE_H_ */
